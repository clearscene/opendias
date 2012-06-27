
<link rel="stylesheet" href="/opendias/style/jquery-ui.css" type="text/css" />
<link rel="stylesheet" href="/opendias/style/jquery.tagsinput.css" type="text/css" />

<script type="text/javascript" src="/opendias/includes/sprintf-0.7-beta1.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery-ui.min.js"></script>
<script type="text/javascript" src="/opendias/includes/jquery.tagsinput.js"></script>
<script type="text/javascript" src="/opendias/includes/local/generic.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.docList.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.docList.js"></script>
<script type="text/javascript" src="/opendias/includes/local/openDias.filter.js.resource"></script>
<script type="text/javascript" src="/opendias/includes/openDias.filter.js"></script>

<div id='filter'>
  <div id='filterOptions'>
    <div id='filterInner'>
      <h3>###### ##:</h3>
      <div class='row'>
        <div class='label'>
          ##### ## OCR ####:
        </div>
        <div class='data'>
          <input id='textSearch' />
        </div>
      </div>
      <div class='row'>
        <div class='label'>
          ### '###### ########':
        </div>
        <div class='data'>
            <input type='checkbox' id='isActionRequired' />
        </div>
      </div>
      <div class='row'>
        <div class='label'>
          ######## ####:
        </div>
        <div class='data'>
            <input id='startDate' /> - <input id='endDate' />
        </div>
      </div>
      <div class='row'>
        <div class='label'>
          ######## ####:
        </div>
        <div class='data'>
          <input id='tags' type='text' class='tags'>
        </div>
      </div>
      <div class='row'>
        <div class='label'>
          &nbsp;
        </div>
        <div class='data'>
          <div class='innerLeftDisplay'>
            <button id='doFilter'>######</button>
          </div>
          <div class='innerRightDisplay'>
            <div id="filterProgress"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
  <div id='filterTab'>
    ###### ### ######
  </div>
</div>

<div id='nodocs'>
  ##### ### ######### ## #### ######. ### #### ##### ### <a href='acquire.html'>####### ###</a> ####.
</div>

<div id='doclist' style='display: none'>

  <h2>######## ####</h2>

  <div class='tableheader'>
    <div id='sortbydocid' class='tabledocid'><span>### ##</span><span class="ui-icon"></span></div>
    <div id='sortbytitle' class='tabletitle'><span>#####</span><span class="ui-icon"></span></div>
    <div id='sortbytype' class='tabletype'><span>####</span><span class="ui-icon"></span></div>
    <div id='sortbydate' class='tabledate'><span>####</span><span class="ui-icon"></span></div>
  </div>

  <div id='results'>
    <div id='binding_block'>
    </div>
    <div id='loading'>#######&nbsp;.&nbsp;.&nbsp;.</div>
  </div>

</div>


