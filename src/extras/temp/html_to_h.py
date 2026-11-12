#!/usr/bin/env python3
"""
Конвертер HTML-файла в .h-файл для Arduino/ESP.

Минифицирует HTML (текст между тегами, CSS внутри <style>, JS внутри <script>)
и оборачивает результат в raw string literal:

    static const char <name>[] PROGMEM = R"(<...>)";

Минификация выполняется посимвольным разбором на токены, а не грубыми
регулярками, поэтому:
  * пробелы удаляются только там, где это безопасно;
  * содержимое строк ('...', "...", `...`), регулярных выражений и
    комментариев не искажается;
  * переводы строк сохраняются только там, где на них опирается ASI
    (автоматическая подстановка ";"), во всех остальных местах они удаляются.

Зависимости: только стандартная библиотека Python 3.
"""

import argparse
import os
import re
import sys


# --------------------------------------------------------------------------- #
#  Утилиты
# --------------------------------------------------------------------------- #

def _is_word_char(ch):
    """Символ, входящий в идентификатор/число (склеивать такие нельзя)."""
    return ch.isalnum() or ch in '_$'


# Ключевые слова, после которых '/' начинает регулярное выражение.
_JS_KEYWORDS_BEFORE_REGEX = {
    'return', 'typeof', 'instanceof', 'in', 'of', 'new', 'delete', 'void',
    'case', 'do', 'else', 'yield', 'await',
}

# Ключевые слова, которые не могут завершать выражение.
_JS_NON_VALUE_WORDS = _JS_KEYWORDS_BEFORE_REGEX | {
    'function', 'if', 'for', 'while', 'switch', 'catch', 'class', 'var',
    'let', 'const', 'try', 'finally', 'throw', 'with',
}

# Пунктуаторы, которые могут завершать выражение.
_JS_VALUE_END_PUNCT = {')', ']', '++', '--'}

# Пунктуаторы, которыми может начинаться новое выражение.
_JS_EXPR_START_PUNCT = {
    '(', '[', '+', '-', '*', '/', '%', '<', '>', '=', '!', '&', '|', '^',
    '~', '?', ':', '++', '--',
}

# Пары "последний символ предыдущего токена + первый символ следующего",
# которые при склейке образовали бы другой токен языка.
_JS_DANGEROUS_PAIRS = {
    '++', '--', '//', '/*', '==', '!=', '<=', '>=', '&&', '||', '=>',
    '+=', '-=', '/=', '<<', '>>', '<!',
}

# Пунктуаторы, которые нужно читать максимально длинными.
_JS_PUNCTUATORS = [
    '>>>=', '...',
    '===', '!==', '**=', '<<=', '>>=', '>>>', '&&=', '||=', '??=',
    '=>', '==', '!=', '<=', '>=', '&&', '||', '??', '?.', '++', '--',
    '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '<<', '>>', '**',
    '{', '}', '(', ')', '[', ']', ';', ',', '<', '>', '+', '-', '*', '/',
    '%', '&', '|', '^', '!', '~', '?', ':', '=', '.', '@', '#',
]


def _js_needs_space(prev, nxt):
    """Нужен ли разделитель между двумя соседними токенами."""
    if not prev or not nxt:
        return False
    a, b = prev[-1], nxt[0]
    if _is_word_char(a) and _is_word_char(b):
        return True
    return (a + b) in _JS_DANGEROUS_PAIRS


def _js_is_value_end(token):
    """Может ли токен завершать выражение (кандидат на конец оператора)."""
    if not token:
        return False
    if token in _JS_VALUE_END_PUNCT:
        return True
    first = token[0]
    if first in '"\'`':
        return True
    if first.isdigit():
        return True
    if _is_word_char(first):
        return token not in _JS_NON_VALUE_WORDS
    return False


def _js_is_expr_start(token):
    """Может ли токен начинать новое выражение."""
    if not token:
        return False
    if token in _JS_EXPR_START_PUNCT:
        return True
    return token[0] in '"\'`' or _is_word_char(token[0])


# Токены, способные ПРОДОЛЖИТЬ выражение после значения. Если после перевода
# строки идёт такой токен — ASI не срабатывает, перевод можно просто удалить.
# Здесь нет '{' и '++/--': после значения '{' начинает отдельный блок, а
# перевод строки запрещает постфиксные ++/-- (restricted production), так что
# в обеих ситуациях ASI подставляет ';'.
_JS_CONTINUABLE_PUNCT = {
    '+', '-', '*', '/', '%', '**', '.', ',', ';', ':', '?',
    '(', ')', '[', ']', '}', '~', '&', '|', '^', '<', '>', '=',
    '&&', '||', '??', '?.', '=>', '==', '!=', '===', '!==', '<=', '>=',
    '<<', '>>', '>>>', '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=',
    '<<=', '>>=', '>>>=', '&&=', '||=', '??=',
}

# Токены, которые продолжают ПРЕДЫДУЩЕЕ выражение, а не начинают новое
# (бинарные операторы-слова). Перевод строки перед ними значим не бывает.
_JS_CONTINUATION_WORDS = {'in', 'of', 'instanceof'}

# Слова, которые продолжают оператор, начатый ДО закрывающей '}'.
_JS_BLOCK_CONTINUE_WORDS = {'else', 'catch', 'finally'}

# Слова, после которых '(' открывает заголовок управляющего оператора.
_JS_CONTROL_WORDS = {'if', 'for', 'while', 'switch', 'catch', 'with'}

# Restricted production: перевод строки после этих слов запрещён, поэтому
# ASI подставит ';' даже если следующий токен мог бы продолжить выражение.
_JS_RESTRICTED_WORDS = {'return', 'throw', 'break', 'continue'}


def _js_asi_needed(prev, token, had_newline, prev_close_control,
                   prev_close_do_body):
    """
    Нужен ли явный ';', если между токенами в исходнике был перевод строки.

    prev_close_control  — prev является ')' , закрывающей заголовок
                          if/for/while/switch/catch: дальше идёт тело
                          оператора, перевод строки семантики не меняет.
    prev_close_do_body  — prev является '}' , закрывающей тело do-цикла:
                          следующее 'while' продолжает тот же оператор.
    """
    if not had_newline or not prev or not token:
        return False
    # Restricted production обрывается переводом строки безусловно.
    if prev in _JS_RESTRICTED_WORDS or prev in ('++', '--'):
        return True
    # Перед продолжением выражения ; не нужен.
    if token in _JS_CONTINUABLE_PUNCT:
        return False
    if prev == '}':
        if token in _JS_BLOCK_CONTINUE_WORDS:
            return False
        if token == 'while' and prev_close_do_body:
            return False
        return True
    if token in _JS_CONTINUATION_WORDS:
        return False
    # Тело if/for/while после закрывающей скобки заголовка.
    if prev_close_control:
        return False
    return _js_is_value_end(prev)


# --------------------------------------------------------------------------- #
#  Читалки литералов
# --------------------------------------------------------------------------- #

def _read_string(src, i):
    """Возвращает индекс сразу за строковым литералом, начинающимся в i."""
    quote = src[i]
    i += 1
    n = len(src)
    while i < n:
        c = src[i]
        if c == '\\':
            i += 2
            continue
        if c == quote:
            return i + 1
        if c == '\n' and quote != '`':
            return i
        i += 1
    return n


def _read_regex(src, i):
    """Возвращает индекс сразу за регулярным выражением, начинающимся в i."""
    i += 1
    n = len(src)
    in_class = False
    while i < n:
        c = src[i]
        if c == '\\':
            i += 2
            continue
        if c == '[':
            in_class = True
        elif c == ']':
            in_class = False
        elif c == '/' and not in_class:
            i += 1
            while i < n and src[i].isalpha():
                i += 1
            return i
        elif c == '\n':
            return i
        i += 1
    return n


# --------------------------------------------------------------------------- #
#  Разбор JavaScript
# --------------------------------------------------------------------------- #

def tokenize_js(src):
    """
    Разбирает JS на значимые токены.
    Возвращает список кортежей (token, был_перевод_строки_перед_ним).
    """
    tokens = []
    i = 0
    n = len(src)
    newline = False
    prev_is_value = False

    while i < n:
        c = src[i]

        if c in ' \t\r\n\f\v':
            if c == '\n':
                newline = True
            i += 1
            continue

        if src.startswith('//', i):
            j = src.find('\n', i)
            i = n if j < 0 else j
            continue

        if src.startswith('/*', i):
            j = src.find('*/', i + 2)
            j = n if j < 0 else j + 2
            if '\n' in src[i:j]:
                newline = True
            i = j
            continue

        if c in '"\'`':
            j = _read_string(src, i)
            tokens.append((src[i:j], newline))
            prev_is_value, newline = True, False
            i = j
            continue

        if c == '/' and not prev_is_value:
            j = _read_regex(src, i)
            tokens.append((src[i:j], newline))
            prev_is_value, newline = True, False
            i = j
            continue

        if _is_word_char(c):
            j = i + 1
            if c.isdigit():
                # число: может содержать точку и экспоненту
                while j < n and (src[j].isalnum() or src[j] in '._'):
                    j += 1
            else:
                while j < n and (src[j].isalnum() or src[j] in '_$'):
                    j += 1
            word = src[i:j]
            tokens.append((word, newline))
            prev_is_value = word not in _JS_KEYWORDS_BEFORE_REGEX
            newline = False
            i = j
            continue

        for punct in _JS_PUNCTUATORS:
            if src.startswith(punct, i):
                tokens.append((punct, newline))
                prev_is_value = punct in _JS_VALUE_END_PUNCT
                newline = False
                i += len(punct)
                break
        else:
            tokens.append((c, newline))
            prev_is_value = False
            newline = False
            i += 1

    return tokens


def minify_js(src):
    """Минификация JS: удаляет комментарии и все лишние пробелы."""
    out = []
    prev = ''
    paren_stack = []          # для '(' — является ли она заголовком if/for/...
    brace_stack = []          # для '{' — является ли она телом do-цикла
    prev_close_control = False
    prev_close_do_body = False

    for token, had_newline in tokenize_js(src):
        if prev and _js_asi_needed(prev, token, had_newline,
                                   prev_close_control, prev_close_do_body):
            # Здесь в исходнике сработал бы ASI — вставляем явный ';'.
            out.append(';')
        elif prev and _js_needs_space(prev, token):
            out.append(' ')
        out.append(token)

        close_control = False
        close_do_body = False
        if token == '(':
            paren_stack.append(prev in _JS_CONTROL_WORDS)
        elif token == ')':
            close_control = bool(paren_stack) and paren_stack.pop()
        elif token == '{':
            brace_stack.append(prev == 'do')
        elif token == '}':
            close_do_body = bool(brace_stack) and brace_stack.pop()
        prev_close_control = close_control
        prev_close_do_body = close_do_body
        prev = token

    return ''.join(out)


# --------------------------------------------------------------------------- #
#  Минификация CSS
# --------------------------------------------------------------------------- #

_CSS_TRIM_AROUND = set('{};:,>~+')


def _split_css_strings(text):
    """Разбивает текст на чередующиеся сегменты: строка, текст, строка..."""
    result = []
    i = 0
    n = len(text)
    buf = []
    while i < n:
        c = text[i]
        if c in '"\'':
            if buf:
                result.append(''.join(buf))
                buf = []
            j = _read_string(text, i)
            result.append(text[i:j])
            i = j
            continue
        buf.append(c)
        i += 1
    if buf:
        result.append(''.join(buf))
    return result


def minify_css(css):
    """Минификация CSS: убирает комментарии и пробелы вокруг разделителей."""
    # 1. Удаление комментариев с сохранением строк.
    parts = []
    i = 0
    n = len(css)
    while i < n:
        c = css[i]
        if c in '"\'':
            j = _read_string(css, i)
            parts.append(css[i:j])
            i = j
            continue
        if css.startswith('/*', i):
            j = css.find('*/', i + 2)
            i = n if j < 0 else j + 2
            continue
        parts.append(c)
        i += 1

    # 2. Сворачиваем пробельные группы и удаляем пробелы вокруг разделителей,
    #    не трогая содержимое строковых литералов.
    out = []
    for chunk in _split_css_strings(''.join(parts)):
        if chunk in '"\'':
            out.append(chunk)
            continue
        for c in chunk:
            if c in ' \t\r\n\f\v':
                # Пробел внутри строки не трогать, здесь он не попадёт.
                # Сворачиваем в один пробел.
                if out and out[-1][-1] not in _CSS_TRIM_AROUND:
                    out.append(' ')
                continue
            if c in _CSS_TRIM_AROUND:
                if out and out[-1] == ' ':
                    out.pop()
                elif out and out[-1][-1] == ' ':
                    out[-1] = out[-1][:-1]
                out.append(c)
                continue
            # Если последний символ — пробел и он в списке — убираем.
            if out and out[-1][-1] in _CSS_TRIM_AROUND:
                pass  # оставим пробел как есть (безопасно)
            out.append(c)

    return ''.join(out)


# --------------------------------------------------------------------------- #
#  Разбор и минификация HTML
# --------------------------------------------------------------------------- #

def _skip_tag(html, start):
    """Индекс сразу за закрывающим '>' тега, с учётом кавычек в атрибутах."""
    i = start + 1
    n = len(html)
    quote = ''
    while i < n:
        c = html[i]
        if quote:
            if c == quote:
                quote = ''
        elif c in '"\'':
            quote = c
        elif c == '>':
            return i + 1
        i += 1
    return n


def _read_attr_value(tag, i):
    """Читает значение атрибута с i. Возвращает (значение, новый индекс)."""
    n = len(tag)
    if i < n and tag[i] in '"\'':
        quote = tag[i]
        j = tag.find(quote, i + 1)
        if j < 0:
            return tag[i:], n
        return tag[i:j + 1], j + 1
    j = i
    while j < n and not tag[j].isspace() and tag[j] != '>':
        j += 1
    return tag[i:j], j


def _normalize_attr_quotes(value):
    """
    Приводит кавычки значения атрибута к одинарным.

    Прошивка ищет в странице литерал "<p id='label'>" (handleGetConfigPage в
    shSRControl.cpp) и подставляет в него адрес JSON-эндпоинта. Если исходный
    HTML отформатирован prettier'ом, атрибуты оказываются в двойных кавычках,
    подстановка не срабатывает, и страница уходит за конфигом по пустому URL,
    получая в ответ HTML вместо JSON.

    Двойные кавычки оставляем, если внутри есть одинарные, иначе атрибут
    рассыплется.
    """
    if len(value) >= 2 and value[0] == '"' and value[-1] == '"' \
            and "'" not in value[1:-1]:
        return "'" + value[1:-1] + "'"
    return value


def normalize_tag(tag):
    """
    Схлопывает пробелы внутри тега, убирает их вокруг '=' и перед '>' и
    приводит кавычки атрибутов к одинарным.
    """
    n = len(tag)
    i = 1
    while i < n and not tag[i].isspace() and tag[i] != '>':
        i += 1
    out = [tag[:i]]

    self_closing = False
    while i < n:
        c = tag[i]
        if c in ' 	\r\n\f\v':
            i += 1
            continue
        if c == '>':
            break
        if c == '/' and tag[i + 1:i + 2] == '>':
            self_closing = True
            i += 2
            continue

        j = i
        while j < n and not tag[j].isspace() and tag[j] not in '=>':
            j += 1
        name = tag[i:j]
        i = j
        if not name:
            i += 1
            continue

        k = i
        while k < n and tag[k].isspace():
            k += 1
        if k < n and tag[k] == '=':
            k += 1
            while k < n and tag[k].isspace():
                k += 1
            value, i = _read_attr_value(tag, k)
            out.append(' ' + name + '=' + _normalize_attr_quotes(value))
        else:
            out.append(' ' + name)

    return ''.join(out) + ('/>' if self_closing else '>')


def minify_html(html):
    """Минификация HTML целиком: текст, теги, <style>, <script>."""
    parts = []
    text_buf = []
    i = 0
    n = len(html)

    def flush_text():
        if text_buf:
            parts.append(('text', re.sub(r'\s+', ' ', ''.join(text_buf))))
            del text_buf[:]

    while i < n:
        if html[i] != '<':
            text_buf.append(html[i])
            i += 1
            continue

        if html.startswith('<!--', i):
            j = html.find('-->', i)
            i = n if j < 0 else j + 3
            continue

        m = re.match(r'<(/?)([a-zA-Z][\w:-]*)', html[i:])
        if not m:
            text_buf.append(html[i])
            i += 1
            continue

        closing = m.group(1) == '/'
        name = m.group(2).lower()
        end = _skip_tag(html, i)
        tag = html[i:end]
        flush_text()
        i = end

        if not closing and name in ('style', 'script') \
                and not tag.rstrip().endswith('/>'):
            closer = re.compile(r'</\s*%s\s*>' % name, re.IGNORECASE)
            m2 = closer.search(html, i)
            if m2:
                # Закрывающий тег допишет форматирование ниже, дублировать его
                # отдельным элементом не нужно.
                parts.append((name, (normalize_tag(tag), html[i:m2.start()])))
                i = m2.end()
                continue

        parts.append(('tag', normalize_tag(tag)))

    flush_text()

    out = []
    for kind, value in parts:
        if kind == 'text':
            out.append(value)
        elif kind == 'tag':
            out.append(value)
        elif kind == 'style':
            out.append('%s%s</style>' % (value[0], minify_css(value[1])))
        elif kind == 'script':
            out.append('%s%s</script>' % (value[0], minify_js(value[1])))

    html = ''.join(out)
    html = re.sub(r'>\s+<', '><', html)
    return html.strip()


# --------------------------------------------------------------------------- #
#  Сборка .h файла
# --------------------------------------------------------------------------- #

def make_raw_string(body):
    """
    Оборачивает строку в C++ raw string literal.
    Если содержимое содержит )" — используется пользовательский разделитель,
    иначе код скомпилируется с ошибкой.
    """
    delimiter = '' if ')"' not in body else 'HTML'
    return 'R"%s(%s)%s"' % (delimiter, body, delimiter)


def html_to_h(html_path, h_path, variable_name, minify=True, check=True):
    with open(html_path, 'r', encoding='utf-8') as f:
        html = f.read()

    original = len(html.encode('utf-8'))
    body = minify_html(html) if minify else html
    size = len(body.encode('utf-8'))

    content = (
        '#pragma once\n'
        '\n'
        '#include <Arduino.h>\n'
        '\n'
        'static const char %s[] PROGMEM =\n'
        '    %s;\n' % (variable_name, make_raw_string(body))
    )

    with open(h_path, 'w', encoding='utf-8') as f:
        f.write(content)

    print('Готово: %s' % h_path)
    print('  Переменная:   %s' % variable_name)
    print('  HTML:         %d байт' % original)
    print('  После миниф.: %d байт' % size)
    if original:
        print('  Экономия:     %.1f%%' % (100 * (1 - size / original)))


def default_var_name(html_path):
    base = os.path.splitext(os.path.basename(html_path))[0]
    base = re.sub(r'[^0-9a-zA-Z_]', '_', base)
    if base and base[0].isdigit():
        base = '_' + base
    return base + '_page'


def main():
    parser = argparse.ArgumentParser(
        description='Конвертер HTML-файла в .h для Arduino/ESP',
    )
    parser.add_argument('html', help='Путь к исходному HTML-файлу')
    parser.add_argument(
        '-o', '--output',
        help='Путь к итоговому .h-файлу (по умолчанию рядом с HTML)',
    )
    parser.add_argument(
        '-n', '--name',
        help='Имя переменной (по умолчанию <имя_файла>_page)',
    )
    parser.add_argument(
        '--no-minify', action='store_true',
        help='Не минифицировать HTML, только обернуть в raw string',
    )
    parser.add_argument(
        '--no-marker-check', action='store_true',
        help='Не проверять наличие подстановочного маркера прошивки',
    )
    args = parser.parse_args()

    html_to_h(
        args.html,
        args.output or os.path.splitext(args.html)[0] + '.h',
        args.name or default_var_name(args.html),
        minify=not args.no_minify,
        check=not args.no_marker_check,
    )
    return 0


if __name__ == '__main__':
    sys.exit(main())
