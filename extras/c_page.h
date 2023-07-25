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
        max-width: 350px;
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
      #tab-btn-3:checked ~ #content-3 {
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
        <h3>Настройки WiFi сети</h3>
        <input type='radio' name='tab-btn' id='tab-btn-1' value='' checked />
        <label for='tab-btn-1'>Режим AP</label>
        <input type='radio' name='tab-btn' id='tab-btn-2' value='' />
        <label for='tab-btn-2'>Режим STA</label>
        <input type='radio' name='tab-btn' id='tab-btn-3' value='' />
        <label for='tab-btn-3' id='apsta'>AP+STA</label>

        <div id='content-1'>
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
        </div>
        <div id='content-2'>
          <h4>Режим STA (WiFi-клиент)</h4>
          <label>Имя сети (SSID)</label><br />
          <input
            type='text'
            name='ssid'
            id='ssid'
            title='Введите имя сети или выберите сеть из списка доступных сетей'
            placeholder='SSID сети'
          />
          <label
            >Доступные WiFi сети
            <a href='#' onclick='getApList()'>(обновить)</a></label
          >
          <select
            size='6'
            onchange="document.getElementById('ssid').value=value"
            id='ap_list'
          ></select>
          <label>Пароль</label><br />
          <input type='text' name='pass' id='pass' placeholder='пароль сети' />
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
          <div id='static_data'>
            <label>IP-адрес</label><br />
            <input type='text' minlength='7' maxlength='15' size='15' id='ip' />
            <label>Шлюз</label><br />
            <input
              type='text'
              minlength='7'
              maxlength='15'
              size='15'
              id='gateway'
            />
            <label>Маска сети</label><br />
            <input
              type='text'
              minlength='7'
              maxlength='15'
              size='15'
              id='mask'
            />
          </div>
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
        <input
          type='button'
          value='Отмена'
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
      function checkSsidValue(input_id) {
        if (
          document.getElementById(input_id).value.length < 8 ||
          document.getElementById(input_id).value.length > 32
        ) {
          alert(
            'Длина SSID должна быть не менее 8 и не более 32 символов! \n\n Разрешенные символы: 0-9, A-Z, a-z, - , _'
          );
          document.getElementById(input_id).focus();
          return false;
        }
        return true;
      }
      function checkPassValue(input_id) {
        if (document.getElementById(input_id).value.length < 8) {
          alert(
            'Длина пароля должна быть не менее 8 символов! \n\n Не разрешаются символы кириллицы и пробелы'
          );
          document.getElementById(input_id).focus();
          return false;
        }
        return true;
      }
      function ipAddressCheck(id) {
        var regEx = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
        if(document.getElementById(id).value.match(regEx))
          {
            return true;
          }
        else
          {
          alert("Please enter a valid ip Address.");
          return false;
          }
      }
      function validate_form() {
        // проверка данных
        if (
          checkSsidValue('ap_ssid') == false ||
          checkPassValue('ap_pass') == false ||
          checkSsidValue('ssid') == false ||
          checkPassValue('pass') == false
        ) {
          return false;
        }
        return true;
      }
      function enableBtnSave() {
        document.getElementById('btn_save').removeAttribute('disabled');
      }
      function showStaticData() {
        if (!document.getElementById('static_ip').checked) {
          document.getElementById('static_data').style.display = 'none';
        } else {
          document.getElementById('static_data').style.display = 'block';
        }
        enableBtnSave();
      }
      function sendData() {
        document.getElementById('btn_save').disabled = "disabled";
        let form_data = {
          ap_ssid: document.getElementById('ap_ssid').value,
          ap_pass: document.getElementById('ap_pass').value,
          ap_ip: document.getElementById('ap_ip').value,
          ap_mask: document.getElementById('ap_mask').value,
          ssid: document.getElementById('ssid').value,
          pass: document.getElementById('pass').value,
          static_ip: document.getElementById('static_ip').checked,
          ip: document.getElementById('ip').value,
          gateway: document.getElementById('gateway').value,
          mask: document.getElementById('mask').value,
          ap_sta: document.getElementById('combo').checked,
        };
        var xhr = new XMLHttpRequest();
        xhr.open('POST', '/setconfig', true);
        xhr.setRequestHeader('Content-Type', 'text/json');
        xhr.send(JSON.stringify(form_data));
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
              .getElementById('ap_mask')
              .setAttribute('value', doc.ap_mask);
            document.getElementById('ssid').setAttribute('value', doc.ssid);
            document.getElementById('pass').setAttribute('value', doc.pass);

            document.getElementById('static_ip').checked = doc.static_ip == 1;
            if (!document.getElementById('static_ip').checked) {
              document.getElementById('static_data').style.display = 'none';
            }
            document.getElementById('ip').setAttribute('value', doc.ip);
            document
              .getElementById('gateway')
              .setAttribute('value', doc.gateway);
            document.getElementById('mask').setAttribute('value', doc.mask);

            if (doc.use_combo != 1)
              document.getElementById('apsta').style.display = 'none';
            if (doc.ap_sta == 1)
              document.getElementById('combo').checked = true;
          }
        };
        request.send();
        getApList();
      }

      document.getElementById('ap_ssid').addEventListener('keyup', function () {
        this.value = this.value.replace(/[^a-zA-Z0-9_/-]/g, '');
      });
      document.getElementById('ssid').addEventListener('keyup', function () {
        this.value = this.value.replace(/[^a-zA-Z0-9_/-]/g, '');
      });
      document.getElementById('ap_pass').addEventListener('keyup', function () {
        this.value = this.value.replace(/[А-Яа-яёЁ/ ]/g, '');
      });
      document
        .getElementById('ap_ssid')
        .addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_pass')
        .addEventListener('input', enableBtnSave);
      document.getElementById('ap_ip').addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_mask')
        .addEventListener('input', enableBtnSave);
      document.getElementById('ssid').addEventListener('input', enableBtnSave);
      document.getElementById('pass').addEventListener('input', enableBtnSave);
      document
        .getElementById('ap_list')
        .addEventListener('change', enableBtnSave);
      document
        .getElementById('static_ip')
        .addEventListener('input', showStaticData);
      document.getElementById('ip').addEventListener('input', enableBtnSave);
      document
        .getElementById('gateway')
        .addEventListener('input', enableBtnSave);
      document.getElementById('mask').addEventListener('input', enableBtnSave);
      document.getElementById('combo').addEventListener('input', enableBtnSave);
      document.addEventListener('DOMContentLoaded', getConfig);
    </script>
  </body>
</html>
)";
