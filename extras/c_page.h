#pragma once

#include <Arduino.h>

static const char config_page[] PROGMEM =
    R"(<!DOCTYPE html>
<html>
  <head>
    <meta content='text/html; charset=UTF-8' http-equiv='content-type' />
    <meta name='VIEWPORT' content='width=device-width, initial-scale=1.0' />
    <style>
      *,
      *::before,
      *::after {
        box-sizing: border-box;
      }
      body {
        margin: 0;
        font-size: 16px;
        font-weight: 400;
        line-height: 1.5;
        color: #212529;
        text-align: left;
        background-color: #fff;
      }
      .tabs {
        font-size: 0;
        max-width: 400px;
        margin-left: auto;
        margin-right: auto;
      }
      .tabs > input[type='radio'] {
        display: none;
      }
      .tabs > div {
        /* скрыть контент по умолчанию */
        display: none;
        border: 2px solid #e0e0e0;
        padding: 10px 15px;
        font-size: 16px;
      }
      .tabs > h3 {
        font-size: 18pt;
      }
      /* отобразить контент, связанный с вабранной радиокнопкой (input type='radio') */
      #tab-btn-1:checked ~ #content-1,
      #tab-btn-2:checked ~ #content-2,
      #tab-btn-3:checked ~ #content-3,
      #tab-btn-4:checked ~ #content-4 {
        display: block;
      }
      .tabs > label {
        display: inline-block;
        text-align: center;
        vertical-align: middle;
        user-select: none;
        background-color: #f5f5f5;
        border: 1px solid #e0e0e0;
        padding: 2px 8px;
        font-size: 16px;
        line-height: 1.5;
        transition: color 0.15s ease-in-out, background-color 0.15s ease-in-out;
        cursor: pointer;
        position: relative;
        top: 1px;
      }
      .tabs > label:not(:first-of-type) {
        border-left: none;
      }
      .tabs > input[type='radio']:checked + label {
        background-color: #cfcfcf;
        border-bottom: 1px solid #fff;
      }
      label {
        margin-left: 10px;
        margin-bottom: 3px;
      }
      input,
      select {
        margin-left: 10px;
        margin-bottom: 3px;
        margin-right: 7px;
        -webkit-box-sizing: content-box;
        -moz-box-sizing: content-box;
        -ms-box-sizing: content-box;
        box-sizing: content-box;
      }
      input[type='text'],
      input[type='password'],
      select {
        display: block;
        width: 300px;
      }
      select {
        font-style: italic;
      }
      .save_btn {
        margin: 15px 10px;
        width: 120px;
      }
      .btn_right {
        margin-left: 20px;
      }
    </style>
  </head>
  <body>
    <form>
      <div class='tabs'>
        <h3>Настройки WiFi модуля</h3>
        <input type='radio' name='tab-btn' id='tab-btn-1' value='' checked />
        <label for='tab-btn-1'>Режим STA</label>
        <input type='radio' name='tab-btn' id='tab-btn-2' value='' />
        <label for='tab-btn-2'>Режим AP</label>
        <input type='radio' name='tab-btn' id='tab-btn-3' value='' />
        <label for='tab-btn-3' id='apsta'>AP+STA</label>
        <input type='radio' name='tab-btn' id='tab-btn-4' value='' />
        <label for='tab-btn-4' id='admin'>Разное</label>

        <div id='content-1'>
          <h4>Режим STA (WiFi-клиент)</h4>
          <label>Имя сети (SSID)</label><br />
          <input
            type='text'
            name='ssid'
            id='ssid'
            title='Введите имя сети или выберите сеть из списка доступных сетей'
            placeholder='SSID сети'
          />
          <label>Пароль</label><br />
          <input type='text' name='pass' id='pass' placeholder='пароль сети' />
          <label
            >Доступные WiFi сети
            <a href='#' onclick='getApList()'>(обновить)</a></label
          >
          <select
            size='6'
            onchange="document.getElementById('ssid').value=value"
            id='ap_list'
          ></select>
          <br />
          <label style='margin-left: 10px'>
            <input
              type='checkbox'
              name='static_ip'
              id='static_ip'
              style='margin-left: 0px'
            />
            Назначить IP вручную</label
          ><br />
          <div id='static_data' style='display: none'>
            <label>IP-адрес</label><br />
            <input type='text' minlength='7' maxlength='15' size='15' id='ip' />
            <label>Маска сети</label><br />
            <input
              type='text'
              minlength='7'
              maxlength='15'
              size='15'
              id='mask'
            />
            <label>Шлюз</label><br />
            <input
              type='text'
              minlength='7'
              maxlength='15'
              size='15'
              id='gateway'
            />
          </div>
        </div>
        <div id='content-2'>
          <h4>Режим AP (точка доступа)</h4>
          <label>Имя точки доступа (SSID)</label><br />
          <input
            type='text'
            name='ap_ssid'
            id='ap_ssid'
            placeholder='SSID точки доступа'
          />
          <label>Пароль</label><br />
          <input
            type='text'
            name='ap_pass'
            id='ap_pass'
            placeholder='пароль точки доступа'
          />
          <br />
          <label>IP-адрес</label><br />
          <input
            type='text'
            minlength='7'
            maxlength='15'
            size='15'
            id='ap_ip'
          />
          <label>Маска сети</label><br />
          <input
            type='text'
            minlength='7'
            maxlength='15'
            size='15'
            id='ap_mask'
          />
          <label>Шлюз</label><br />
          <input
            type='text'
            minlength='7'
            maxlength='15'
            size='15'
            id='ap_gateway'
          />
        </div>
        <div id='content-3'>
          <h4>Комбинированный режим (AP+STA)</h4>
          <label style='margin-left: 10px'>
            <input
              type='checkbox'
              name='combo'
              id='combo'
              style='margin-left: 0px'
            />
            Включить комбинированный режим</label
          >
        </div>
        <div id='content-4'>
          <h4>Управление модулем</h4>
          <label style='margin-left: 10px'>
            <input
              type='checkbox'
              name='use_adm_pass'
              id='use_adm_pass'
              style='margin-left: 0px'
            />
            Доступ к настройкам по паролю</label
          ><br />
          <div id='adm_pass' style='display: none'>
            <label>Логин</label><br />
            <input
              type='text'
              name='a_name'
              id='a_name'
              placeholder='логин для входа в настройки'
            />
            <label>Пароль</label><br />
            <input
              type='text'
              name='a_pass'
              id='a_pass'
              placeholder='пароль для входа в настройки'
            />
          </div>
        </div>
        <input
          type='button'
          value='Назад'
          class='save_btn'
          onclick="window.open('/', '_self', false);"
        />
        <input
          type='button'
          id='btn_save'
          class='save_btn btn_right'
          value='Сохранить'
          onclick='sendData()'
          disabled
        />
      </div>
    </form>
    <div></div>
    <script>
      function checkSsidValue(i_id, is_empty) {
        var tag = document.getElementById(i_id);
        if (is_empty && tag.value.length == 0) {
          return true;
        }
        var res =
          i_id == 'a_name'
            ? tag.value.length >= 5
            : tag.value.length >= 8 && tag.value.length <= 32;
        if (!res) {
          tag.focus();
          showErrorPage(i_id);
          if (i_id == 'a_name') {
            alert(
              'Длина имени должна быть не менее 5 символов! \n\n Разрешенные символы: 0..9, A..Z, a..z, - , _'
            );
          } else {
            alert(
              'Длина SSID должна быть не менее 8 и не более 32 символов! \n\n Разрешенные символы: 0..9, A..Z, a..z, - , _'
            );
          }
        }
        return res;
      }
      function checkPassValue(i_id, is_empty) {
        var tag = document.getElementById(i_id);
        if (is_empty && tag.value.length == 0) {
          return true;
        }
        var res = tag.value.length >= 8;
        if (!res) {
          tag.focus();
          showErrorPage(i_id);
          alert(
            'Длина пароля должна быть не менее 8 символов! \n\n Не разрешаются символы кириллицы и пробелы'
          );
        }
        return res;
      }
      function ipAddressCheck(i_id) {
        var tag = document.getElementById(i_id);
        var regEx =
          /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
        if (tag.value.match(regEx)) {
          return true;
        } else {
          tag.focus();
          showErrorPage(i_id);
          alert(
            'Неправильно введен IP адрес!\n\n IP адрес должен иметь вид XXX.XXX.XXX.XXX \n\n где XXX - число в интервале 0..255'
          );
          return false;
        }
      }
      function validate_form() {
        var res =
          checkSsidValue('ap_ssid', false) &&
          checkPassValue('ap_pass', true) &&
          ipAddressCheck('ap_ip') &&
          ipAddressCheck('ap_mask') &&
          ipAddressCheck('ap_gateway');
        if (res) {
          res = checkSsidValue('ssid', true) && checkPassValue('pass', true);
        }
        if (res && document.getElementById('static_ip').checked) {
          res =
            ipAddressCheck('ip') &&
            ipAddressCheck('gateway') &&
            ipAddressCheck('mask');
        }
        if (res && document.getElementById('use_adm_pass').checked) {
          res =
            checkSsidValue('a_name', false) && checkPassValue('a_pass', false);
        }
        return res;
      }
      function showErrorPage(id) {
        if (
          id == 'ssid' ||
          id == 'pass' ||
          id == 'ip' ||
          id == 'gateway' ||
          id == 'mask'
        ) {
          document.getElementById('tab-btn-1').checked = true;
        } else if (
          id == 'ap_ssid' ||
          id == 'ap_pass' ||
          id == 'ap_ip' ||
          id == 'ap_gateway' ||
          id == 'ap_mask'
        ) {
          document.getElementById('tab-btn-2').checked = true;
        } else {
          document.getElementById('tab-btn-4').checked = true;
        }
        document.getElementById(id).focus();
      }
      function enableBtnSave() {
        document.getElementById('btn_save').removeAttribute('disabled');
      }
      function showStaticData() {
        showDiv('static_ip', 'static_data', true);
      }
      function showAdmPassData() {
        showDiv('use_adm_pass', 'adm_pass', true);
      }
      function showDiv(id1, id2, btnEnbl) {
        var x = document.getElementById(id1).checked ? 'block' : 'none';
        document.getElementById(id2).style.display = x;
       if(btnEnbl) {enableBtnSave();}
      }
      function sendData() {
        if (validate_form()) {
          document.getElementById('btn_save').disabled = 'disabled';
          let form_data = {
            ap_ssid: document.getElementById('ap_ssid').value,
            ap_pass: document.getElementById('ap_pass').value,
            ap_ip: document.getElementById('ap_ip').value,
            ap_gateway: document.getElementById('ap_gateway').value,
            ap_mask: document.getElementById('ap_mask').value,
            ssid: document.getElementById('ssid').value,
            pass: document.getElementById('pass').value,
            static_ip: document.getElementById('static_ip').checked,
            ip: document.getElementById('ip').value,
            gateway: document.getElementById('gateway').value,
            mask: document.getElementById('mask').value,
            ap_sta: document.getElementById('combo').checked,
            use_adm_pass: document.getElementById('use_adm_pass').checked,
            a_name: document.getElementById('a_name').value,
            a_pass: document.getElementById('a_pass').value,
          };
          var xhr = new XMLHttpRequest();
          xhr.open('POST', '/setconfig', true);
          xhr.setRequestHeader('Content-Type', 'text/json');
          xhr.onreadystatechange = function () {document.body.innerHTML = this.responseText;};
          xhr.send(JSON.stringify(form_data));
        }
      }
      function getApList() {
        var sel = document.getElementById('ap_list');
        sel.disabled = true;
        while (sel.options.length > 0) {
          sel.remove(0);
        }
        var request = new XMLHttpRequest();
        request.open('GET', '/getaplist', true);
        request.onload = function () {
          if (request.readyState == 4 && request.status == 200) {
            var config_str = request.responseText;
            var doc = JSON.parse(config_str);
            if (doc.aps.length > 0) {
              sel.disabled = false;
              for (var i = 0; i < doc.aps.length; i++) {
                var opt = new Option(doc.aps[i], doc.aps[i]);
                sel.options[i] = opt;
                if (
                  sel.options[i].value.toUpperCase() ===
                  document.getElementById('ssid').value.toUpperCase()
                ) {
                  sel.options[i].selected = true;
                }
              }
            }
          }
        };

        request.send();
      }
      function getConfig() {
        var request = new XMLHttpRequest();
        request.open('GET', '/getconfig', true);
        request.onload = function () {
          if (request.readyState == 4 && request.status == 200) {
            var config_str = request.responseText;
            var doc = JSON.parse(config_str);
            document
              .getElementById('ap_ssid')
              .setAttribute('value', doc.ap_ssid);
            document
              .getElementById('ap_pass')
              .setAttribute('value', doc.ap_pass);
            document.getElementById('ap_ip').setAttribute('value', doc.ap_ip);
            document
              .getElementById('ap_gateway')
              .setAttribute('value', doc.ap_gateway);
            document
              .getElementById('ap_mask')
              .setAttribute('value', doc.ap_mask);
            document.getElementById('ssid').setAttribute('value', doc.ssid);
            document.getElementById('pass').setAttribute('value', doc.pass);

            document.getElementById('static_ip').checked = doc.static_ip == 1;
            showDiv('static_ip', 'static_data', false);
            document.getElementById('ip').setAttribute('value', doc.ip);
            document
              .getElementById('gateway')
              .setAttribute('value', doc.gateway);
            document.getElementById('mask').setAttribute('value', doc.mask);

            if (doc.use_combo != 1) {
              document.getElementById('apsta').style.display = 'none';
            }
            document.getElementById('combo').checked = doc.ap_sta == 1;
            document.getElementById('use_adm_pass').checked =
              doc.use_adm_pass == 1;
            showDiv('use_adm_pass', 'adm_pass', false);
            document.getElementById('a_name').setAttribute('value', doc.a_name);
            document.getElementById('a_pass').setAttribute('value', doc.a_pass);
          }
        };
        request.send();
        getApList();
      }
      function keyUpBySsid() {
        this.value = this.value.replace(/[^a-zA-Z0-9_/-]/g, '');
      }
      function keyUpByPass() {
        this.value = this.value.replace(/[А-Яа-яёЁ/ ]/g, '');
      }
      function keyUpByIP() {
        this.value = this.value.replace(/[^0-9.]/g, '');
      }
      function setApIpData() {
        setGateway('ap_gateway', 'ap_ip', false);
        setMask('ap_mask');
      }
      function setStaIpData() {
        setGateway('gateway', 'ip', true);
        setMask('mask');
      }
      function setMask(i_id) {
        if (document.getElementById(i_id).value.length == 0) {
          document.getElementById(i_id).setAttribute('value', '255.255.255.0');
        }
      }
      function setGateway(i_id, o_id, sub) {
        if (document.getElementById(i_id).value.length == 0) {
          var s = document.getElementById(o_id).value;
          if (sub) {
            s = s.substr(0, s.lastIndexOf('.') + 1) + '1';
          }
          document.getElementById(i_id).setAttribute('value', s);
        }
      }
      document.getElementById('ap_ssid').addEventListener('keyup', keyUpBySsid);
      document.getElementById('ssid').addEventListener('keyup', keyUpBySsid);
      document.getElementById('ap_pass').addEventListener('keyup', keyUpByPass);
      document.getElementById('pass').addEventListener('keyup', keyUpByPass);
      document.getElementById('ip').addEventListener('keyup', keyUpByIP);
      document.getElementById('gateway').addEventListener('keyup', keyUpByIP);
      document.getElementById('mask').addEventListener('keyup', keyUpByIP);
      document.getElementById('ap_ip').addEventListener('keyup', keyUpByIP);
      document
        .getElementById('ap_gateway')
        .addEventListener('keyup', keyUpByIP);
      document.getElementById('ap_mask').addEventListener('keyup', keyUpByIP);
      document.getElementById('a_name').addEventListener('keyup', keyUpBySsid);
      document.getElementById('a_pass').addEventListener('keyup', keyUpByPass);
      document
        .getElementById('ap_ssid')
        .addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_pass')
        .addEventListener('input', enableBtnSave);
      document.getElementById('ap_ip').addEventListener('input', enableBtnSave);
      document.getElementById('ap_ip').addEventListener('blur', setApIpData);
      document
        .getElementById('ap_gateway')
        .addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_gateway')
        .addEventListener('focus', setApIpData);
      document
        .getElementById('ap_mask')
        .addEventListener('input', enableBtnSave);
      document.getElementById('ap_mask').addEventListener('focus', setApIpData);
      document.getElementById('ssid').addEventListener('input', enableBtnSave);
      document.getElementById('pass').addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_list')
        .addEventListener('change', enableBtnSave);
      document
        .getElementById('static_ip')
        .addEventListener('input', showStaticData);
      document.getElementById('ip').addEventListener('input', enableBtnSave);
      document.getElementById('ip').addEventListener('blur', setStaIpData);
      document
        .getElementById('gateway')
        .addEventListener('input', enableBtnSave);
      document
        .getElementById('gateway')
        .addEventListener('focus', setStaIpData);
      document.getElementById('mask').addEventListener('input', enableBtnSave);
      document.getElementById('mask').addEventListener('focus', setStaIpData);
      document.getElementById('combo').addEventListener('input', enableBtnSave);
      document
        .getElementById('use_adm_pass')
        .addEventListener('input', showAdmPassData);
      document
        .getElementById('a_name')
        .addEventListener('input', enableBtnSave);
      document
        .getElementById('a_pass')
        .addEventListener('input', enableBtnSave);
      document.addEventListener('DOMContentLoaded', getConfig);
    </script>
  </body>
</html>
)";
