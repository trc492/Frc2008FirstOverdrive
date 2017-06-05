<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output indent="yes" method="html"/>
  <xsl:param name="sourceDir">html</xsl:param>
  <xsl:template match="/">
    <HTML>
      <HEAD>
        <meta name="GENERATOR" content="Microsoft HTML Help Workshop 4.1"/>
		<xsl:comment>Sitemap 1.0</xsl:comment>
      </HEAD>
      <BODY>
        <UL>
            <xsl:apply-templates select="Contents/owner">
				<xsl:sort select="@name"/>
			</xsl:apply-templates>
        </UL>
      </BODY>
    </HTML>
  </xsl:template>
  
  
<xsl:template match="owner">
<xsl:choose>
    <xsl:when test="@name != 'NULL'">
        <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
        <OBJECT type="text/sitemap">
            <xsl:apply-templates select="Overview[@name=../@name]">
		        <xsl:sort select="@name"/>
	        </xsl:apply-templates>
     <!--       <param name="Name">
            <xsl:attribute name="value"><xsl:value-of select="@name"/></xsl:attribute>
            </param> -->
            <!-- remove links from owners for now until we have method
            of linking them to the right page -->
            <!--
            <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@name"/>_overview.html</xsl:attribute>
            </param>
            -->
        </OBJECT>
        <xsl:text disable-output-escaping="yes">&#x20;&#xD;&#xA;</xsl:text>			  
            <xsl:apply-templates select="owner">
		        <xsl:sort select="@name"/>
	        </xsl:apply-templates>
            <xsl:apply-templates select="Namespace">
			    <xsl:sort select="@name"/>
		    </xsl:apply-templates>
<!--        <xsl:apply-templates select="Overview">
		    <xsl:sort select="@name"/>
	    </xsl:apply-templates> -->
    </xsl:when>
    <xsl:otherwise>
        <xsl:apply-templates select="owner">
		    <xsl:sort select="@name"/>
	    </xsl:apply-templates>
        <xsl:apply-templates select="Namespace">
			<xsl:sort select="@name"/>
		</xsl:apply-templates>
<!--        <xsl:apply-templates select="Overview">
		    <xsl:sort select="@name"/>
	    </xsl:apply-templates> -->
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
  
<xsl:template match="Overview">
    <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
    <OBJECT type="text/sitemap">
        <param name="Name">
            <xsl:attribute name="value">
        	    <xsl:choose>
                    <xsl:when test="boolean(Docs/member/@displayname)">
                        <xsl:value-of select="Docs/member/@displayname"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="@name"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
        </param>
        <param name="Local">
        <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="(substring(@id,3))"/>_overview.html</xsl:attribute>
        </param>
    </OBJECT>
    <xsl:text disable-output-escaping="yes">&#xD;&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Namespace">
<xsl:choose>
    <xsl:when test="@name != 'T:NULL'">
        <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
        <OBJECT type="text/sitemap">
            <param name="Name">
            <xsl:attribute name="value"><xsl:value-of select="(substring(@name,3))"/> [Namespace]</xsl:attribute>
            </param>
            <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="(substring(@name,3))"/>_ns.html</xsl:attribute>
            </param>
        </OBJECT>
        <xsl:text disable-output-escaping="yes">&#x20;&#xD;&#xA;</xsl:text>			  
            <xsl:apply-templates select="Class">
				<xsl:sort select="@name"/>
			</xsl:apply-templates>
            <xsl:apply-templates select="owner">
	    	    <xsl:sort select="@name"/>
	        </xsl:apply-templates>
    </xsl:when>
    <xsl:otherwise>
        <xsl:apply-templates select="Class">
			<xsl:sort select="@name"/>
		</xsl:apply-templates>
        <xsl:apply-templates select="owner">
		    <xsl:sort select="@name"/>
	    </xsl:apply-templates>
    </xsl:otherwise>
  </xsl:choose>
  </xsl:template>
  
  <xsl:template match="Class">
    <xsl:choose>
        <xsl:when test="@name != 'NULL'">
            <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
            <OBJECT type="text/sitemap">
                <param name="Name">
                <xsl:attribute name="value"><xsl:value-of select="@name"/><xsl:if test="../@name != 'T:NULL'"> [Class of <xsl:value-of select="(substring(../@name,3))"/>]</xsl:if></xsl:attribute>
                </param>
                <param name="Local">
                <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html</xsl:attribute>
                </param>
            </OBJECT>
	        <xsl:text disable-output-escaping="yes">&#xD;&#xA;</xsl:text>
                <xsl:apply-templates select="Method">
			        <xsl:sort select="@name"/>
                </xsl:apply-templates>
                <xsl:apply-templates select="Methods">
			        <xsl:sort select="@name"/>
                </xsl:apply-templates>
                <xsl:apply-templates select="owner">
		            <xsl:sort select="@name"/>
	            </xsl:apply-templates>
        </xsl:when>
        <xsl:otherwise>
            <xsl:apply-templates select="Method">
	            <xsl:sort select="@name"/>
            </xsl:apply-templates>
            <xsl:apply-templates select="Methods">
	            <xsl:sort select="@name"/>
            </xsl:apply-templates>
            <xsl:apply-templates select="owner">
	    	    <xsl:sort select="@name"/>
    	    </xsl:apply-templates>
        </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="Method">
    <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
    <OBJECT type="text/sitemap">
    <xsl:choose>
      <xsl:when test="@name[.='#ctor']">
        <param name="Name">
			<xsl:attribute name="value">Constructor<xsl:if test="../@name != 'NULL'"> [Constructor of <xsl:value-of select="../@name"/>]</xsl:if></xsl:attribute>
        </param>
        <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.Constructor.html</xsl:attribute>
        </param>
      </xsl:when>
      <xsl:otherwise>
        <param name="Name">
            <xsl:attribute name="value"><xsl:value-of select="@name"/><xsl:if test="../@name != 'NULL'"> [Method of <xsl:value-of select="../@name"/>]</xsl:if></xsl:attribute>
        </param>
        <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html</xsl:attribute>
        </param>
      </xsl:otherwise>
    </xsl:choose>
	</OBJECT>
	<xsl:text disable-output-escaping="yes">&#xD;&#xA;</xsl:text>
  </xsl:template>
  <xsl:template match="Methods">
    <xsl:text disable-output-escaping="yes">&#xD;&#xA;&lt;LI&gt;</xsl:text>
    <OBJECT type="text/sitemap">
    <xsl:choose>
      <xsl:when test="@name[.='#ctor']">
        <param name="Name">
        <xsl:attribute name="value">Constructor<xsl:if test="../@name != 'NULL'"> [Constructor of <xsl:value-of select="../@name"/>]</xsl:if></xsl:attribute>
        </param>
        <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.Constructor.html</xsl:attribute>
        </param>
      </xsl:when>
      <xsl:otherwise>
        <param name="Name">
            <xsl:attribute name="value"><xsl:value-of select="@name"/><xsl:if test="../@name != 'NULL'"> [Method of <xsl:value-of select="../@name"/>]</xsl:if></xsl:attribute>
        </param>
        <param name="Local">
            <xsl:attribute name="value"><xsl:value-of select="$sourceDir"/>\<xsl:value-of select="@ns"/>.<xsl:value-of select="@name"/>.html</xsl:attribute>
        </param>
      </xsl:otherwise>
    </xsl:choose>
    </OBJECT>
	<xsl:text disable-output-escaping="yes">&#xD;&#xA;</xsl:text>
</xsl:template>

</xsl:stylesheet>