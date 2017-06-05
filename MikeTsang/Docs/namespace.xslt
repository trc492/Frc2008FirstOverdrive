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
<TITLE><xsl:value-of select="substring(Namespace/@name,3)"/> Namespace</TITLE>
<link rel="stylesheet" type="text/css"><xsl:attribute name="href"><xsl:value-of select="$cssFile"/></xsl:attribute></link>
</HEAD>
<BODY>
    <xsl:apply-templates select="Namespace"/>
</BODY>
</HTML>
</xsl:template>

<xsl:template match="Namespace">
<h1><xsl:value-of select="substring(@name,3)"/></h1>
<dt><h3>Classes</h3></dt>
<dd>
<table width="100%">
	<tr>
		<th>Class</th><th>Description</th>
	</tr>
	<xsl:apply-templates select="Class"/>
</table>
</dd>	
</xsl:template>


<xsl:template match="Class">
<tr valign="top">
	<td><a><xsl:attribute name="href"><xsl:value-of select="substring(@id,3)"/>.html</xsl:attribute><xsl:value-of select="@name"/></a></td>
	<td>
	<xsl:apply-templates select="Docs/member"/>
	</td>
</tr>
</xsl:template>

<xsl:template match="member">
	<xsl:apply-templates select="summary"/>
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

	<xsl:template match="listheader">
		<tr>
			<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
		</tr>
	</xsl:template>

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

	<xsl:template match="term">
		<td>
			<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
		</td>
	</xsl:template>

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

<xsl:template match="img"><xsl:text disable-output-escaping="yes">&lt;img src="</xsl:text><xsl:value-of select="@src"/><xsl:text disable-output-escaping="yes">"&gt;</xsl:text></xsl:template>

</xsl:stylesheet>