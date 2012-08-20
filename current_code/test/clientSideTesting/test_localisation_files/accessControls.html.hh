<h2>###### ########</h2>

<div class='accessControl'>

<h3>## ########</h3>
<p>##### ###### #### ####### #########.</p>

<h4>### ### ###### ## ########</h4>
<form action='/opendias/dynamic' method='post'>
<input name='action' type='hidden' value='controlAccess' />
<input name='submethod' type='hidden' value='addLocation' />
<div id='locationSet'>
  <div class='row'>
    <div class='label'>#######:</div>
    <div class='data'><input name='address' />&nbsp;<span>## 127.0.0.1 ## 192.168.%</span></div>
  </div>
  <div class='row'>
    <div class='label'>Role:</div>
    <div class='data'>
      <select name='role'>
        <option value='1'>##### (### ## ### ##########)</option>
        <option value='2'>#### (### ## ##########, ###### ###### ########)</option>
        <option value='3'>#### (### #### ######## ####)</option>
        <option value='4'>### (### ### ######### ####)</option>
      </select>
    </div>
  </div>
  <div class='row'>
    <div class='label'>&nbsp;</div>
    <div class='data'><input type='submit' name='addButton' value='###' /></div>
  </div>
</div>
</form>

<h4>####### ###### ## ########</h4>
<form action='/opendias/dynamic' method='post'>
<input name='action' type='hidden' value='controlAccess' />
<input name='method' type='hidden' value='removeLocation' />
<div id='locationList'>
  <div class='row'>
    <div class='label'>&nbsp;</div>
    <div class='data'>
      <table id='locationTable' class='stdTable'>
        <thead>
          <tr>
            <th>########</th>
            <th>####</th>
            <th>&nbsp;</th>
          </tr>
        </thead>
        <tbody>
        </tbody>
      </table>
    </div>
  </div>
</div>
</form>
<div class='cclear'></div>


<br /><br />
<!--
<div class='gOut'>
</div>

<h3>## ########/########</h3>
<p>##### ###### ### ####### #####<span id='withLocation'><strong>, ## ###### ## ### ######## ## ########</strong></span>.</p>

<h4>### ###### ## ######## ### ########</h4>
<form action='/opendias/dynamic' method='post'>
<input name='action' type='hidden' value='controlAccess' />
<input name='method' type='hidden' value='addUser' />
<div id='locationSet'>
  <div class='row'>
    <div class='label'>####:</div>
    <div class='data'><input name='user' /></div>
  </div>
  <div class='row'>
    <div class='label'>########:</div>
    <div class='data'><input type='password' name='password' /></div>
  </div>
  <div class='row'>
    <div class='label'>####:</div>
    <div class='data'>
      <select name='role'>
        <option value='1'>##### (### ## ### ##########)</option>
        <option value='2'>#### (### ## ##########, ###### ###### ########)</option>
        <option value='3'>#### (### #### ######## ####)</option>
        <option value='4'>### (### ### ######### ####)</option>
      </select>
    </div>
  </div>
  <div class='row'>
    <div class='label'>&nbsp;</div>
    <div class='data'><input type='submit' name='addButton' value='###' /></div>
  </div>
</div>
</form>

<h4>####### ###### ## ######## ### ########</h4>
<form action='/opendias/dynamic' method='post'>
<input name='action' type='hidden' value='controlAccess' />
<input name='method' type='hidden' value='removeUser' />
<div id='locationList'>
  <div class='row'>
    <div class='label'>&nbsp;</div>
    <div class='data'>
      <table id='userTable' class='stdTable'>
        <thead>
          <tr>
            <th>####</th>
            <th>####</th>
            <th>&nbsp;</th>
          </tr>
        </thead>
        <tbody>
        </tbody>
      </table>
    </div>
  </div>
</div>
</form>
<div class='cclear'></div>
-->

</div>
<script type="text/javascript" src="/opendias/includes/local/generic.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.getAccessDetails.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.getAccessDetails.js"></script>
