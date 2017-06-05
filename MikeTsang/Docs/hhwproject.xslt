<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output omit-xml-declaration="yes" method="text"/>
	<xsl:param name="sourceDir">html</xsl:param>
	<xsl:param name="title"></xsl:param>
	<xsl:param name="fileName"></xsl:param>
<xsl:template match="/">
[OPTIONS]
Binary TOC=Yes
Compatibility=1.1 or later
Compiled file=<xsl:value-of select="$fileName"/>.chm
Contents file=<xsl:value-of select="$fileName"/>.hhc
Default Window=Nav Pane
Default topic=<xsl:value-of select="$sourceDir"/>\namespaces.html
Display compile progress=No
Full-text search=Yes
Index file=<xsl:value-of select="$fileName"/>.hhk
Language=0x409 English (United States)
Title=<xsl:value-of select="$title"/>

[WINDOWS]
Nav Pane="<xsl:value-of select="$title"/>","<xsl:value-of select="$fileName"/>.hhc","<xsl:value-of select="$fileName"/>.hhk","<xsl:value-of select="$sourceDir"/>\namespaces.html","<xsl:value-of select="$sourceDir"/>\namespaces.html",,,,,0x63420,300,0x386e,[25,50,1025,650],,,,,,,0

[FILES]
<xsl:value-of select="$sourceDir"/>\namespaces.html
            <xsl:apply-templates select="Contents/owner">
				<xsl:sort select="@name"/>
			</xsl:apply-templates>
</xsl:template>

<xsl:template match="owner">
    <!-- remove links from owners for now until we have method
    of linking them to the right page -->
    <!--
	<xsl:if test="@name != 'NULL'">
            <xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@name"/>_overview.html
    </xsl:if>
    -->
    <xsl:apply-templates select="Namespace">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
    <xsl:apply-templates select="owner">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
    <xsl:apply-templates select="Overview">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
</xsl:template>

<xsl:template match="Overview">
        <xsl:value-of select="$sourceDir"/>\<xsl:value-of select="(substring(@id,3))"/>_overview.html
</xsl:template>

<xsl:template match="Namespace">
	<xsl:if test="@name != 'T:NULL'">
            <xsl:value-of select="$sourceDir"/>\<xsl:value-of select="(substring(@name,3))"/>_ns.html
    </xsl:if>
    <xsl:apply-templates select="Class">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
    <xsl:apply-templates select="Overview">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
</xsl:template>

  <xsl:template match="Class">
    <xsl:if test="@name != 'NULL'">
        <xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html
    </xsl:if>
    <xsl:apply-templates select="Method">
	    <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:apply-templates select="Methods">
	    <xsl:sort select="@name"/>
    </xsl:apply-templates>
    <xsl:apply-templates select="Overview">
		<xsl:sort select="@name"/>
	</xsl:apply-templates>
  </xsl:template>

  <xsl:template match="Method">
    <xsl:choose>
      <xsl:when test="@name[.='#ctor']">
<xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.Constructor.html
      </xsl:when>
      <xsl:otherwise>
<xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  <xsl:template match="Methods">
    <xsl:choose>
      <xsl:when test="@name[.='#ctor']">
<xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.Constructor.html
      </xsl:when>
      <xsl:otherwise>
<xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html
      </xsl:otherwise>
    </xsl:choose>
</xsl:template>
</xsl:stylesheet>