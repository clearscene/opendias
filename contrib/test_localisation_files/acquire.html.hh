
<script type="text/javascript" src="/opendias/includes/jquery-ui.min.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.canvas-loader.js"></script>
<link rel="stylesheet" href="/opendias/style/jquery-ui.css" type="text/css" media="screen" /> 
<style>
  .ui-progressbar-value { background-image: url(/opendias/images/jquery-ui/pbar-ani.png); }
  #tabs {font-size: 11px;}
</style>

<h2>####### ### ########</h2>

<div id='cached' class='warning ui-corner-all'>
  <h3>####### ### ### #######</h3>
  <p>## ### #### ##### ####### #### #########, ### #### ######### ### #### ####### 
    # ######## #######. #############, #### ### #### ###### ### ####### ## # ####. 
    ########, ### ####### #### ##### ### ### ####### ### ### #### ### #### 
    ########## #######.</p>
  <p>### ### ### ## ### ### ## ### ###### ########, ### ###### #### ####### ##### 
    #### ##### ###### #########, ## #### #########.</p>
  <p>#############, ### ##### ####### #### ####, ##### #### ####### ## ### ######
    #########.</p>
</div>

<div id="tabs">

  <ul id='tabList'>
    <li><a href="#info">###########</a></li>
    <li><a href="#office">######## ######</a></li>
  </ul>

  <div id="info">
    <h3>######### # ### ########</h3>
    <p>### #### ###### ## ### # ### ######## ## #### #######.</p>
    <p>## ### ##### ###, ### ### ### ##### ## #### #####. 
      #########: PDF ## ODF ######### ### ######, ######s #### ### 
      #### ####### ## ############# #### ########.</p>
    <p>### ########## #### ######### ####### ######## ##### #######.</p>
    <p id='scanning'><span id='loading'></span>&nbsp;######## ### #######...</p>
  </div>

  <div id="office">
    <h3>######## #####t</h3>
    <p>###### # PDF, ODF ######## ## JPEG ##### ### ###### #### #### ######.</p>
    <form action='/opendias/dynamic' method='post' enctype="multipart/form-data">
      <input name='action' type='hidden' value='uploadfile' />
      <div class='row'>
        <div class='label'>######## ## ######:</div>
        <div class='data'><input name='uploadfile' type='file' /></div>
      </div>
      <div class='row'>
        <div class='label'>PDF ########</div>
        <div class='data'><input type='radio' name='ftype' value='PDF' /></div>
      </div>
      <div class='row'>
        <div class='label'>ODF ########</div>
        <div class='data'><input type='radio' name='ftype' value='ODF' /></div>
      </div>
      <div class='row'>
        <div class='label'>JPEG #####</div>
        <div class='data'><input type='radio' name='ftype' value='jpg' /></div>
      </div>
      <div class='row'>
        <div class='label'>&nbsp;</div>
        <div class='data'><input id='uploadButton' type='submit' value='######' /></div>
      </div>
    </form>
  </div>

  <div id='scannerTemplate'>
    <div class='row'>
      <div class='label'>&nbsp;</div>
      <div class='data'>
        <h3 id='title_DEVICE'>scanner title</h3>
        <p></p>
        <input id='deviceid_DEVICE' type='hidden' />
      </div>
    </div>
<!--
    <div class='row'>
      <div class='label'>######:</div>
      <div class='data'><select id='format_DEVICE'></select></div>
    </div>
-->
    <input id='format_DEVICE' type='hidden' value='Grey Scale' />
    <div class='row'>
      <div class='label' title='### ###### ## ##### #### #### ## ### ######## ### ### ##### ## ####'>#####:</div>
      <div class='data'>
        <input id='pages_DEVICE' type='hidden' value='1' />
        <div id='pagesDisplay_DEVICE' class='innerLeftDisplay'>1 #####</div>
        <div class='innerRightDisplay'><div id='pagesSlider_DEVICE'></div></div>
      </div>
    </div>
    <div class='row'>
      <div class='label' title='### ########## ### ####### ###### ### #### ######## #### ########. ### ######, ### ###### #######. ### ####, ### ######, ### ###### #### ### ###### ########## #### ##.'>##########:</div>
      <div class='data'>
        <input id='resolution_DEVICE' type='hidden' />
        <div id='resolutionDisplay_DEVICE' class='innerLeftDisplay'>x dpi</div>
        <div class='innerRightDisplay'>
          <div id='resolutionSlider_DEVICE'></div>
        </div>
      </div>
    </div>
    <div class='row'>
      <div class='label' title='##### #### ## ### ######## ## #### ### ####### ##### ### ###### ### #### ## ### ########. ### #### #### ## #### ## ###### ## #### #### #####.'>###:</div>
      <div class='data'>
        <div class='innerLeftDisplay'>
        <div class='data'><select id='ocr_DEVICE'>
                          <option value="-">## ###</option>
                          <option value="nld">#####</option>
                          <option value="eng" selected='selected'>#######</option>
                          <option value="fra">######</option>
                          <option value="deu">######</option>
                          <option value="ita">#######</option>
                          <option value="por">##########</option>
                          <option value="spa">#######</option>
                          <option value="vie">##########</option>
                        </select></div>
        </div>
        <div class='innerRightDisplay'>
          <div class='resolutionQuality' title='########## ## ### #### ########## ### ######. #### ###### ### ##### #### ### #### #######.'>
            <div id='resolutionGood_DEVICE' class='resolutionQualityGood' title='### ##### ######## ########## #### ### ######.'></div>
          </div>
        </div>
      </div>
    </div>
    <div class='row'>
      <div class='label' title='### #### ### ### ### ## ### #### ## ####. ## #### ######## # ######## ####### #### ### ### ###### #### #### ####### ########. #### ##### #### ### #####.'>#### ######:</div>
      <div class='data'>
        <input id='length_DEVICE' type='hidden' value='100' />
        <div id='lengthDisplay_DEVICE' class='innerLeftDisplay'>100 %</div>
        <div class='innerRightDisplay'>
          <div id='lengthSlider_DEVICE'></div>
        </div>
      </div>
    </div>
    <div class='row'>
      <div class='label'>&nbsp;</div>
      <div class='data'>
        <div class='innerLeftDisplay'>
          <button id='scanButton_DEVICE'>####</button>
        </div>
        <div class='innerRightDisplay'>
          <div id="status_DEVICE"></div>
          <div id="progressbar_DEVICE"></div>
        </div>
      </div>
    </div>
  </div>
</div>

<script type="text/javascript" src="/opendias/includes/local/generic.resource"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.acquire.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.acquire.js"></script>
