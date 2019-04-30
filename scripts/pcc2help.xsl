<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <!--

     Stylesheet to format PCC2 help into HTML

     The generated HTML closely (but not completely) resembles the
     original hand-written PCC2 help file, pcc2-overview.html.

   -->
 <xsl:output method="html" doctype-public="-//W3C//DTD HTML 4.0 Transitional//EN" />

 <xsl:param name="whoami"></xsl:param>

 <xsl:template match="/help">
  <xsl:comment>Documentation for PCC2, HTML generated from help file.</xsl:comment>
  <html>
   <head>
    <title>PCC2 Help</title>
    <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
     <style type="text/css" title="Standard Style">
   body {
     background: #000;
     color: #ccc;
     font-family: sans-serif;
     max-width: 60em;
     margin-left: auto;
     margin-right: auto;
   }
   h1 {
     color: #fff;
     border-bottom: solid #fff 2px;
     border-right: solid #fff 2px;
     background: #444;
     padding: 0.3em;
   }
   h1:before {
     content: ">> ";
   }
   b {
     color: #fff;
   }
   h2 {
     background: #044;
     color: #8ff;
     border: solid #4cc 2px;
     padding: 0.2em;
   }
   h3 {
     color: #8ff;
     border-bottom: solid #4cc 2px;
     padding: 0.2em;
   }
   a:link {
     color: #aae;
     text-decoration: none;
   }
   a:visited {
     color: #ccf;
     text-decoration: none;
   }
   /* This makes oh-so-neat keycaps, even better than in PCC 1.x :-) */
   kbd {
     font-variant: small-caps;
     border-style: solid;
     border-width: 1px 2px 2px 1px;
     border-color: #888 #888 #666 #666;
     /* -moz-border-radius: 2px 2px 2px 2px; */
     padding: 0px 2px 0px 2px;
     background-color: #ccc;
     color: #000;
   }
   /* Avoid consuming too much vertical space for the TOC */
   ul.toc li ul li {
     display: inline;
     white-space: nowrap;
   }
   ul.toc li ul li.first:before {
     content: ' [ ';
   }
   ul.toc li ul li:before {
     content: ' | ';
   }
   ul.toc li ul li.last:after {
     content: ' ] ';
   }
   /* first-in-priority table */
   tr.fp td {
     border-top: solid #888 1px;
   }
   /* Colors */
   .colorstatic     { }
   .colorgreen      { color: #0c0; }
   .colorgreen b    { color: #0f0; }
   .colorgreen kbd  { background-color: #cfc; }
   .coloryellow     { color: #cc0; }
   .coloryellow b   { color: #ff0; }
   .coloryellow kbd { background-color: #ffc; }
   .colorred        { color: #c00; }
   .colorred b      { color: #f00; }
   .colorred kbd    { background-color: #fcc; }
   .colorwhite      { color: #fff; }
   .colorblue       { color: #44c; }
   .colorblue b     { color: #66f; }
   .colorblue kbd   { background-color: #ccf; }
   .colordim        { color: #666; }
   .colordim b      { color: #ccc; }
   .colordim kbd    { background-color: #aaa; }
   .small           { font-size: 80%; }
   .big             { font-size: 125%; }
   /* TOC link */
   a.toclink {
     display: block;
     float: right;
     font-size: 10pt;
     color: #aae;
   }
   /* Script Examples */
   pre.x {
     margin-left: 4em;
     margin-right: 4em;
     padding: 0.2em;
     background-color: #000020;
     border: solid #000040 1px;
     text-decoration: none;
   }
    </style>
   </head>
   <body>
    <h1 id="toc">PCC2 Help</h1>

    <p>This is the PCC2 help in one big HTML file.
     This help is also available within the program, using <kbd>Alt</kbd>-<kbd>H</kbd>.</p>

    <ul class="toc">
     <xsl:for-each select="/help/page">
      <xsl:call-template name="makeTOC" />
     </xsl:for-each>
    </ul>

    <xsl:apply-templates />
   </body>
  </html>
 </xsl:template>

 <xsl:template match="page">
  <xsl:apply-templates />
 </xsl:template>

 <!-- h1/2/3: contains text -->
 <xsl:template match="h1">
  <h2>
   <xsl:if test="name(..) = 'page'">
    <xsl:attribute name="id"><xsl:value-of select="../@id" /></xsl:attribute>
   </xsl:if>
   <a class="toclink" href="#toc">[Top]</a>
   <xsl:apply-templates />
  </h2>
 </xsl:template>

 <xsl:template match="h2">
  <h3><xsl:apply-templates /></h3>
 </xsl:template>

 <xsl:template match="h3">
  <h4><xsl:apply-templates /></h4>
 </xsl:template>

 <!-- paragraph: contains anything -->
 <xsl:template match="p">
  <p><xsl:apply-templates /></p>
 </xsl:template>

 <!-- lists -->
 <xsl:template match="ul">
  <ul>
   <xsl:for-each select="li">
    <li><xsl:apply-templates /></li>
   </xsl:for-each>
  </ul>
 </xsl:template>

 <xsl:template match="ol">
  <ol>
   <xsl:for-each select="li">
    <li><xsl:apply-templates /></li>
   </xsl:for-each>
  </ol>
 </xsl:template>

 <xsl:template match="dl">
  <dl>
   <xsl:for-each select="di">
    <dt><xsl:value-of select="@term" /></dt>
    <dd><xsl:apply-templates /></dd>
   </xsl:for-each>
  </dl>
 </xsl:template>

 <xsl:template match="kl">
  <ul>
   <xsl:for-each select="ki">
    <li>
     <xsl:call-template name="formatKey">
      <xsl:with-param name="key" select="@key" />
     </xsl:call-template>: <xsl:apply-templates />
    </li>
   </xsl:for-each>
  </ul>
 </xsl:template>

 <!-- soft line break -->
 <xsl:template match="br">
  <br />
 </xsl:template>

 <!-- images -->
 <xsl:template match="img">
  <!-- FIXME -->
 </xsl:template>

 <!-- tables -->
 <xsl:template match="table">
  <table>
   <xsl:if test="@align">
    <xsl:attribute name="align"><xsl:value-of select="@align" /></xsl:attribute>
   </xsl:if>
   <xsl:if test="not(@align)">
    <xsl:attribute name="align">center</xsl:attribute>
   </xsl:if>
   <xsl:for-each select="tr">
    <tr><xsl:apply-templates /></tr>
   </xsl:for-each>
  </table>
  <xsl:if test="@align">
   <div style="clear:both;"></div>
  </xsl:if>
 </xsl:template>

 <xsl:template match="td">
  <td>
   <xsl:if test="@align">
    <xsl:attribute name="align"><xsl:value-of select="@align" /></xsl:attribute>
   </xsl:if>
   <xsl:if test="@width">
    <xsl:attribute name="width"><xsl:value-of select="16*@width" /></xsl:attribute>
   </xsl:if>
   <xsl:apply-templates/>
  </td>
 </xsl:template>

 <xsl:template match="th">
  <th>
   <xsl:if test="@align">
    <xsl:attribute name="align"><xsl:value-of select="@align" /></xsl:attribute>
   </xsl:if>
   <xsl:if test="@width">
    <xsl:attribute name="width"><xsl:value-of select="16*@width" /></xsl:attribute>
   </xsl:if>
   <xsl:apply-templates/>
  </th>
 </xsl:template>

 <xsl:template match="tn">
  <td>
   <xsl:if test="@align">
    <xsl:attribute name="align"><xsl:value-of select="@align" /></xsl:attribute>
   </xsl:if>
   <xsl:if test="not(@align)">
    <xsl:attribute name="align">right</xsl:attribute>
   </xsl:if>
   <xsl:if test="@width">
    <xsl:attribute name="width"><xsl:value-of select="16*@width" /></xsl:attribute>
   </xsl:if>
   <xsl:apply-templates/>
  </td>
 </xsl:template>

 <!-- text formatting -->
 <xsl:template match="b|em|tt|u">
  <xsl:element name="{name()}">
   <xsl:apply-templates />
  </xsl:element>
 </xsl:template>

 <xsl:template match="kbd|key">
  <xsl:call-template name="formatKey">
   <xsl:with-param name="key" select="text()" />
  </xsl:call-template>
 </xsl:template>

 <xsl:template match="font[@color]">
  <span class="color{@color}">
   <xsl:apply-templates />
  </span>
 </xsl:template>

 <xsl:template match="small">
  <span class="small">
   <xsl:apply-templates />
  </span>
 </xsl:template>

 <xsl:template match="big">
  <span class="big">
   <xsl:apply-templates />
  </span>
 </xsl:template>

 <xsl:template match="pre[@class='ccscript']">
  <pre class="x"><xsl:apply-templates /></pre>
 </xsl:template>

 <xsl:template match="pre">
  <pre><xsl:apply-templates /></pre>
 </xsl:template>

 <xsl:template match="a">
  <xsl:choose>
   <xsl:when test="substring(@href, 1, 4) = 'int:'">
    <!-- Link to interpreter documentation -->
    <xsl:if test="$whoami = 'int'">
     <a href="#{@href}"><xsl:apply-templates /></a>
    </xsl:if>
    <xsl:if test="not($whoami = 'int')">
     <a href="pcc2interpreter.html#{@href}"><xsl:apply-templates /></a>
    </xsl:if>
   </xsl:when>
   <xsl:otherwise>
    <!-- Link to main documentation -->
    <xsl:if test="$whoami = 'int'">
     <a href="pcc2help.html#{@href}"><xsl:apply-templates /></a>
    </xsl:if>
    <xsl:if test="not($whoami = 'int')">
     <a href="#{@href}"><xsl:apply-templates /></a>
    </xsl:if>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:template>


 <!-- formatKey(key) formats a keycap in the same way as richhelp.cc
      does it: split up at punctuation and put individual items in
      <kbd> tags. Instead of a loop, this uses recursion. -->

 <!-- formatKey(key) -->
 <xsl:template name="formatKey">
  <xsl:param name="key" />
  <xsl:call-template name="formatKeyKey">
   <xsl:with-param name="key" select="$key" />
   <xsl:with-param name="start" select="1" />
   <xsl:with-param name="i" select="2" />
  </xsl:call-template>
 </xsl:template>

 <!-- formatKeyKey(key, start, i) -->
 <xsl:template name="formatKeyKey">
  <xsl:param name="key" />
  <xsl:param name="start" />
  <xsl:param name="i" />
  <xsl:choose>
   <!-- end reached? -->
   <xsl:when test="$i > string-length($key)">
    <kbd><xsl:value-of select="substring($key, $start)" /></kbd>
   </xsl:when>
   <!-- punctuation reached? -->
   <xsl:when test="contains('-+/,;: ', substring($key, $i, 1))">
    <kbd><xsl:value-of select="substring($key, $start, $i - $start)" /></kbd>
    <xsl:call-template name="formatKeyPunct">
     <xsl:with-param name="key" select="$key" />
     <xsl:with-param name="start" select="$i" />
     <xsl:with-param name="i" select="$i + 1" />
     <xsl:with-param name="punct" select="' '" />
    </xsl:call-template>
   </xsl:when>
   <!-- punctuation reached? -->
   <xsl:when test="contains('.', substring($key, $i, 1))">
    <kbd><xsl:value-of select="substring($key, $start, $i - $start)" /></kbd>
    <xsl:call-template name="formatKeyPunct">
     <xsl:with-param name="key" select="$key" />
     <xsl:with-param name="start" select="$i" />
     <xsl:with-param name="i" select="$i + 1" />
     <xsl:with-param name="punct" select="'. '" />
    </xsl:call-template>
   </xsl:when>
   <!-- still in key name -->
   <xsl:otherwise>
    <xsl:call-template name="formatKeyKey">
     <xsl:with-param name="key" select="$key" />
     <xsl:with-param name="start" select="$start" />
     <xsl:with-param name="i" select="$i + 1" />
    </xsl:call-template>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:template>

 <!-- formatKeyPunct(key, start, i) -->
 <xsl:template name="formatKeyPunct">
  <xsl:param name="key" />
  <xsl:param name="start" />
  <xsl:param name="i" />
  <xsl:param name="punct" />
  <xsl:choose>
   <!-- end reached? -->
   <xsl:when test="$i > string-length($key)">
    <xsl:value-of select="substring($key, $start)" />
   </xsl:when>
   <!-- key reached? -->
   <xsl:when test="not(contains($punct, substring($key, $i, 1)))">
    <xsl:value-of select="substring($key, $start, $i - $start)" />
    <xsl:call-template name="formatKeyKey">
     <xsl:with-param name="key" select="$key" />
     <xsl:with-param name="start" select="$i" />
     <xsl:with-param name="i" select="$i + 1" />
    </xsl:call-template>
   </xsl:when>
   <!-- still in punctuation -->
   <xsl:otherwise>
    <xsl:call-template name="formatKeyPunct">
     <xsl:with-param name="key" select="$key" />
     <xsl:with-param name="start" select="$start" />
     <xsl:with-param name="i" select="$i + 1" />
     <xsl:with-param name="punct" select="$punct" />
    </xsl:call-template>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:template>

 <!-- TOC generation -->
 <xsl:template name="makeTOC">
  <li>
   <xsl:if test="position()=1"><xsl:attribute name="class">first</xsl:attribute></xsl:if>
   <xsl:if test="position()=last()"><xsl:attribute name="class">last</xsl:attribute></xsl:if>
   <xsl:if test="last()=1"><xsl:attribute name="class">first last</xsl:attribute></xsl:if>
   <a href="#{@id}"><xsl:value-of select="h1" /></a>
   <xsl:if test="page">
    <ul>
     <xsl:for-each select="page">
      <xsl:call-template name="makeTOC" />
     </xsl:for-each>
    </ul>
   </xsl:if>
   </li>
  </xsl:template>

</xsl:stylesheet>
