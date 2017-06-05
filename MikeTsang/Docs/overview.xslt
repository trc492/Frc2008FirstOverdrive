<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt"
	xmlns:msnia="urn:microsoft-msn-ia"
>
<xsl:param name="cssFile">msdn.css</xsl:param>
<xsl:param name="systemBaseUrl">ms-help://MS.NETFrameworkSDK/cpref/html/frlrf</xsl:param>
<xsl:param name="systemBaseUrlExt">htm</xsl:param>

<msxsl:script implements-prefix="msnia" language="C#"> 
<![CDATA[ 
string readableName(string attribName){

    string valName;
    int nColon = attribName.IndexOf(':');
    valName = attribName.Substring(nColon+1);
    
    while(valName.StartsWith("NULL."))
    {
        valName = valName.Substring(5);
    }
    
    valName = valName.Trim('.');
    return valName;
}

]]> </msxsl:script>

<xsl:template match="/">
<HTML>
<HEAD>
<TITLE><xsl:value-of select="Overview/@name"/></TITLE>
<link rel="stylesheet" type="text/css"><xsl:attribute name="href"><xsl:value-of select="$cssFile"/></xsl:attribute></link>
<style type="text/css">
  div.tabit { margin-left:30px; }
</style>
</HEAD>
<BODY>
	<dl>
		<dt>
			<br/>
		</dt>
		<dd>
			<xsl:apply-templates select="Overview"/>
		</dd>
	</dl>
</BODY>
</HTML>
</xsl:template>

<xsl:template match="Overview">
<h1>
    <xsl:choose>
        <xsl:when test="boolean(Docs/member/@displayname)">
            <xsl:value-of select="Docs/member/@displayname"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="@name"/>
        </xsl:otherwise>
    </xsl:choose>
</h1>
<p><xsl:apply-templates select="Docs/member"/></p>
	<xsl:if test="//seealso">
		<dl>
			<dt>
				<h4>See Also</h4>
			</dt>
			<dd>
				<xsl:apply-templates select="//seealso"/>
			</dd>
		</dl>
	</xsl:if>
	<xsl:if test="Docs/member/@source">
		<p>
			<font color="#888888">Source File: <xsl:value-of select="Docs/member/@source"/>
		</font>
		</p>
	</xsl:if>
	<p><font color="#888888">Reference ID: <xsl:value-of select="@name"/></font>
    </p>

		
</xsl:template>

<xsl:template match="member">
    <xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="summary"><p><xsl:apply-templates select="text()|*[name() != 'seealso']"/></p></xsl:template>

<xsl:template match="param">
    <dt><i><xsl:value-of select="@name"/></i></dt>
	<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
</xsl:template>

<xsl:template match="value">
    <h4>Value</h4>
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="returns">
    <h4>Returns</h4>
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="exception">
    <dt><i><xsl:value-of select="@cref"/></i></dt>
	<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
</xsl:template>

<xsl:template match="remarks">
    <h4>Remarks</h4>
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="example">
    <h4>Example</h4>
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="seealso">
    <xsl:if test="@cref"><a>
        <xsl:choose>
			<xsl:when test="contains(@cref,'System.') and not(contains(//Class/@id,'System.'))">
				<xsl:attribute name="href"><xsl:value-of select="concat($systemBaseUrl, substring(translate(@cref, '.', ''),3), 'MembersTopic.',$systemBaseUrlExt)" /></xsl:attribute>	
			</xsl:when>
			<xsl:when test="@tref">
			    <a>
				    <xsl:attribute name="href">
					    <xsl:value-of select="substring(@tref,3)"/>_overview.html
				    </xsl:attribute>
    			    <xsl:value-of select="@displayname"/>
			    </a>
    		</xsl:when>
			<xsl:otherwise>
				<xsl:attribute name="href"><xsl:value-of select="substring(@cref,3)"/>.html</xsl:attribute>	
			</xsl:otherwise>
        </xsl:choose>
	<xsl:value-of select="msnia:readableName(@cref)" />
	</a>  
	</xsl:if>
</xsl:template>

<xsl:template match="text()"><xsl:value-of select="."/></xsl:template>

<xsl:template match="para"><p><xsl:apply-templates select="text()|*[name() != 'seealso']"/></p></xsl:template>

<xsl:template match="code"><pre><xsl:value-of select="."/></pre></xsl:template>

<xsl:template match="see">
    <xsl:choose>
        <xsl:when test="@langword"><code><xsl:value-of select="@langword"/></code></xsl:when>
        <xsl:when test="@cref"><a>
			<xsl:choose>
			    <xsl:when test="contains(@cref,'System.') and not(contains(//Class/@id,'System.'))">
					<xsl:attribute name="href"><xsl:value-of select="concat($systemBaseUrl, substring(translate(@cref, '.', ''),3), 'MembersTopic.',$systemBaseUrlExt)" /></xsl:attribute>	
				</xsl:when>
				<xsl:otherwise>
					<xsl:attribute name="href"><xsl:value-of select="substring(@cref,3)"/>.html</xsl:attribute>	
				</xsl:otherwise>
			</xsl:choose>
			<xsl:value-of select="msnia:readableName(@cref)" /></a>
		</xsl:when>
		<xsl:when test="@tref">
			<a>
				<xsl:attribute name="href">
					<xsl:value-of select="substring(@tref,3)"/>_overview.html
				</xsl:attribute>
    			<xsl:value-of select="@displayname"/>
			</a>
		</xsl:when>
    </xsl:choose>
</xsl:template>

<xsl:template match="list">
	<xsl:choose>
		<xsl:when test="starts-with(@type,'bullet')">
			<ul>
				<xsl:apply-templates select="text()|*[name() != 'seealso' and name() != 'listheader']"/>
			</ul>
		</xsl:when>
		<xsl:when test="starts-with(@type,'number')">
			<ol>
				<xsl:apply-templates select="text()|*[name() != 'seealso' and name() != 'listheader']"/>
			</ol>
		</xsl:when>
		<xsl:otherwise>
			<p>
				<table>
					<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
				</table>
			</p>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="listheader"><tr><xsl:apply-templates select="text()|*[name() != 'seealso']"/></tr></xsl:template>

<xsl:template match="item">
	<xsl:choose>
		<xsl:when test="boolean(parent::list[@type='bullet' or @type='number'])">
			<li>
				<xsl:apply-templates select="text()|*[name() != 'seealso' and name() != 'term']"/>
			</li>
		</xsl:when>
		<xsl:otherwise>
			<tr>
				<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
			</tr>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="term"><td><xsl:apply-templates select="text()|*[name() != 'seealso']"/></td></xsl:template>

<xsl:template match="description">
	<xsl:choose>
		<xsl:when test="boolean(parent::*/parent::list[@type='bullet' or @type='number'])">
			<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
		</xsl:when>
		<xsl:otherwise>
			<td>
				<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
			</td>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="c"><code><xsl:apply-templates select="text()|*[name() != 'seealso']"/></code></xsl:template>

<xsl:template match="paramref"><i><xsl:value-of select="@name"/></i></xsl:template>

<xsl:template match="img"><xsl:text disable-output-escaping="yes">&lt;img src="</xsl:text><xsl:value-of select="@src"/><xsl:text disable-output-escaping="yes">"</xsl:text>
    <xsl:text disable-output-escaping="yes"> width="</xsl:text><xsl:value-of select="@width"/><xsl:text disable-output-escaping="yes">"</xsl:text>
    <xsl:text disable-output-escaping="yes"> border="</xsl:text><xsl:value-of select="@border"/><xsl:text disable-output-escaping="yes">"</xsl:text>
    <xsl:text disable-output-escaping="yes"> align="</xsl:text><xsl:value-of select="@align"/><xsl:text disable-output-escaping="yes">"</xsl:text>
    <xsl:text disable-output-escaping="yes">&gt;</xsl:text></xsl:template>

<xsl:template match="h1"><xsl:text disable-output-escaping="yes">&lt;h1&gt;</xsl:text><xsl:value-of select="."/><xsl:text disable-output-escaping="yes">&lt;/h1&gt;</xsl:text></xsl:template>

<xsl:template match="br"><xsl:text disable-output-escaping="yes">&lt;br&gt;</xsl:text></xsl:template>
<xsl:template match="li"><xsl:text disable-output-escaping="yes">&lt;li&gt;</xsl:text></xsl:template>

<xsl:template match="ul"><ul><xsl:apply-templates select="text()|*[name() != 'seealso']"/></ul></xsl:template>

</xsl:stylesheet>