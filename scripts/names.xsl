<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<!-- Generate list of names from Doxygen Tagfile -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" />
  <xsl:template match="/">
    <!-- Member variables -->
    <xsl:for-each select="//compound[@kind='class']/member[@kind='variable' and not(@static='yes')]">
      <xsl:value-of select="name/text()" /><xsl:text>:member-variable&#10;</xsl:text>
    </xsl:for-each>

    <!-- Member functions -->
    <xsl:for-each select="//compound[@kind='class']/member[@kind='function']">
      <xsl:if test="type/text() != '' and substring(name/text(), 1, 1) != '~' and substring(name/text(), 1, 8) != 'operator'">
        <xsl:value-of select="name/text()" /><xsl:text>:member-function&#10;</xsl:text>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>
</xsl:stylesheet>
