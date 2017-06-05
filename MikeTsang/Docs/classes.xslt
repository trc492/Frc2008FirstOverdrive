<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:msxsl="urn:schemas-microsoft-com:xslt"
	xmlns:msnia="urn:microsoft-msn-ia"
>
<xsl:param name="cssFile">msdn.css</xsl:param>
<xsl:param name="systemBaseUrl">ms-help://MS.NETFrameworkSDK/cpref/html/frlrf</xsl:param>
<xsl:param name="systemBaseUrlExt">htm</xsl:param>
<xsl:param name="baseNs"></xsl:param>

<msxsl:script implements-prefix="msnia" language="C#"> 
<![CDATA[ 
string ApplyTypeLinks(XPathNodeIterator nodes,string systemBaseUrl,string baseUrlExt,string baseNs){
	XPathNavigator xpn;
	XmlNode xn,parentNode;
	string className = null,type;
	bool isSystem = false;
	string output = "<a href=\"{0}\" class=\"typeLink\">{1}</a>";
	bool array = false;
	
	xpn = nodes.Current;
	xn = ((IHasXmlNode)xpn).GetNode();
	type = xn.InnerText;
	parentNode = xn.ParentNode;
	while (className == null){
		if (parentNode.Attributes["name"] != null &&
			parentNode.Attributes["name"].InnerText.IndexOf("T:") > -1){
			className = parentNode.Attributes["name"].InnerText;
		}else if (parentNode.Attributes["id"] != null &&
			parentNode.Attributes["id"].InnerText.IndexOf("T:") > -1){
			className = parentNode.Attributes["id"].InnerText;
		}
		parentNode = parentNode.ParentNode;	
	}
	className = className.Replace("T:","");
	if (baseNs == String.Empty){
		isSystem = className.IndexOf("System.") == 0;
	}else{
		if (type.IndexOf(baseNs) == -1){
			isSystem = false;
		}else{
			isSystem = true;
		}
	}
	
	if (type.IndexOf("[]") > -1){
		type = type.Replace("[]","");
		array = true;
	}
	if (type.IndexOf("System.") > -1 && !isSystem){
		output = String.Format(output,systemBaseUrl + type.Replace(".","") + "MembersTopic." + baseUrlExt,type);
	}else{
		output = String.Format(output,type + ".html",type);
	}
	output += array ? "[] ": " ";
	return output;
}
bool IsAttributeTrue(XmlNode node, string attribName){

	if (node == null)
		return false;
	
	XmlAttribute attrib = node.Attributes[attribName];
	if (attrib == null)
		return false;
	
	return attrib.InnerText == Boolean.TrueString;
}

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
<TITLE><xsl:value-of select="Class/@name"/> Class</TITLE>
<link rel="stylesheet" type="text/css"><xsl:attribute name="href"><xsl:value-of select="$cssFile"/></xsl:attribute></link>
</HEAD>
<BODY>
	<dl>
		<dt>
			<br/>
		</dt>
		<dd>
		    <xsl:apply-templates select="Class"/>
		 </dd>
	</dl>

</BODY>
</HTML>
</xsl:template>

<xsl:template match="Class">
<xsl:choose>
	<xsl:when test="name(parent::node()) != 'Class'">
	<h1><xsl:apply-templates select="Assembly"/>
		<xsl:choose>
			<xsl:when test="substring(@id , 3, 5) = 'NULL.'">
                <xsl:value-of select="substring(@id,8)"/>			
			</xsl:when>
			<xsl:otherwise>
                <xsl:value-of select="substring(@id,3)"/>			
			</xsl:otherwise>
		</xsl:choose>
	</h1>
	<dl>
	<table>

	<tr valign="top">
		<td>
			<xsl:apply-templates select="Docs/member"/>	
			<xsl:if test="Methods or Method">
			<dt><h4>Methods</h4></dt>
			<dd>
				<table>
					<tr><th>Name</th><th>Description</th></tr>
					<xsl:apply-templates select="Methods"/>
					<xsl:apply-templates select="Method"/>
				</table>
			</dd>
			</xsl:if>
			
			<xsl:if test="Fields or Field">
			<dt><h4>Fields</h4></dt>
			<dd>
				<dl>
				<xsl:apply-templates select="Fields"/>
				<xsl:apply-templates select="Field"/>		
				</dl>
			</dd>
			</xsl:if>
			
			<xsl:if test="Properties or Property">
			<dt><h4>Properties</h4></dt>
			<dd>
				<dl>
				<xsl:apply-templates select="Properties"/>
				<xsl:apply-templates select="Property"/>
				</dl>
			</dd>
			</xsl:if>
			<xsl:if test="Event">
			<dt><h4>Properties</h4></dt>
			<dd>
				<dl>
				<xsl:apply-templates select="Event"/>
				</dl>
			</dd>
			
			</xsl:if>
		</td>
	</tr>
		<xsl:if test="//seealso">
		<tr>
			<td>
				<dt>
					<h4>See Also</h4>
				</dt>
				<dd>
					<xsl:if test="substring(@id , 3, 5) != 'NULL.'">
						<a><xsl:attribute name="href"><xsl:value-of select="substring(substring-before(@id,concat('.',@name)),3)"/>_ns.html</xsl:attribute><xsl:value-of select="substring(substring(substring-before(@id,@name),3),0,string-length(substring(substring-before(@id,@name),3)))"/></a>\
					</xsl:if>
					<xsl:apply-templates select="//seealso"/>
				
				</dd>
			</td>
		</tr>
		</xsl:if>
		<xsl:if test="Assembly">
	<tr>
		<td>
		<xsl:apply-templates select="Assembly/BaseType"/>
		<xsl:apply-templates select="Assembly/Interfaces"/>
		<xsl:apply-templates select="Assembly/AsmName"/>
		</td>
	</tr>
	</xsl:if>
    <xsl:if test="Class">
		<tr>
		<td>
			<h4>Classes / Enums</h4>
			<xsl:apply-templates select="Class"/>
		</td>
		</tr>
	</xsl:if>
	</table>
	</dl>
		<xsl:if test="Docs/member/@source">
			<p>
				<font color="#888888">
					Source File: <xsl:value-of select="Docs/member/@source"/>
				</font>
			</p>
		</xsl:if>

	</xsl:when>
	<xsl:otherwise>
		<a><xsl:attribute name="href"><xsl:value-of select="substring(@id,3)"/>.html</xsl:attribute><xsl:value-of select="substring(@id,3)"/></a><br/>
	</xsl:otherwise>
</xsl:choose>	

</xsl:template>

<xsl:template match="Methods">
<tr>
<xsl:choose>
	<xsl:when test="@name[. = '#ctor']">
	<td><a><xsl:attribute name="href"><xsl:value-of select="substring(../@id,3)"/>.Constructor.html</xsl:attribute>Constructor</a></td>
	</xsl:when>
	<xsl:otherwise>
	<td><a><xsl:attribute name="href"><xsl:value-of select="substring(@id,3)"/>.html</xsl:attribute><xsl:value-of select="@name"/></a></td>
	</xsl:otherwise>
</xsl:choose>
<td>(<xsl:value-of select="count(Method)"/> Overloads)</td>
</tr>
</xsl:template>

<xsl:template match="Method">
<tr>
<xsl:choose>
	<xsl:when test="@name[. = '#ctor']">
	<td><a><xsl:attribute name="href"><xsl:value-of select="substring(../@id,3)"/>.Constructor.html</xsl:attribute>Constructor</a></td>
	<td>creates a new instance of this class</td>
	</xsl:when>
	<xsl:otherwise>
	<td><a><xsl:attribute name="href"><xsl:value-of select="substring(../@id,3)"/>.<xsl:value-of select="@name"/>.html</xsl:attribute><xsl:value-of select="@name"/></a></td>
	<td><xsl:apply-templates select="Docs/member/summary"/></td>
	</xsl:otherwise>
</xsl:choose>
</tr>

</xsl:template>

<xsl:template match="Properties">
	<xsl:apply-templates select="Property"/>
</xsl:template>

<xsl:template match="Property">
	<dt><xsl:apply-templates select="Assembly"/><xsl:apply-templates select="Assembly/Type"/> <i><xsl:value-of select="@name"/></i></dt>
	<dd><xsl:apply-templates select="Docs/member"/></dd>
</xsl:template>

<xsl:template match="Fields">
	<xsl:apply-templates select="Field"/>	
</xsl:template>

<xsl:template match="Field">
	<dt><xsl:if test="../Assembly/@enum[. ='False']"><xsl:apply-templates select="Assembly"/><xsl:apply-templates select="Assembly/Type"/> </xsl:if><i><xsl:value-of select="@name"/></i></dt>
	<dd><xsl:apply-templates select="Docs/member"/></dd>
</xsl:template>
<xsl:template match="Event">
	<dt><i><xsl:value-of select="@name"/></i></dt>
	<dd><xsl:apply-templates select="Docs/member"/></dd>
</xsl:template>

<xsl:template match="member">
	<xsl:apply-templates select="summary"/>
	<xsl:apply-templates select="remarks"/>
	<xsl:apply-templates select="example"/>
	<xsl:if test="permission">
		<dt><h4>Permission</h4></dt>
		<dd><xsl:apply-templates select="permission"/></dd>
	</xsl:if>

	<!--<xsl:if test="seealso">
		<h4>See Also</h4>
		<xsl:apply-templates select="seealso"/>
	</xsl:if>-->
</xsl:template>
<xsl:template match="summary"><xsl:apply-templates select="text()|*[name() != 'seealso']"/></xsl:template>

<xsl:template match="param">
	<dl>
		<dt><i><xsl:value-of select="@name"/></i></dt>
		<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
	</dl>
</xsl:template>

<xsl:template match="value">
    <h4>Value</h4>
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/>
</xsl:template>

<xsl:template match="returns">
	<dt><h4>Return Value</h4></dt>
	<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
</xsl:template>

<xsl:template match="exception">
	<dl>
		<dt><i><xsl:value-of select="substring(@cref,3)"/></i></dt>
		<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
	</dl>
</xsl:template>

<xsl:template match="remarks">
	<dt><h4>Remarks</h4></dt>
	<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
</xsl:template>

<xsl:template match="example">
	<dt><h4>Example</h4></dt>
	<dd><xsl:apply-templates select="text()|*[name() != 'seealso']"/></dd>
</xsl:template>

<xsl:template match="permission">
	<xsl:apply-templates select="text()|*[name() != 'seealso']"/><br/>
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

<xsl:template match="para"><br/><xsl:apply-templates select="text()|*[name() != 'seealso']"/></xsl:template>

<xsl:template match="code"><pre><xsl:apply-templates select="text()|*[name() != 'seealso']"/></pre></xsl:template>

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

<xsl:template match="Assembly">
<xsl:if test="@interface[. = 'False'] and @public[. = 'True']">public </xsl:if>
<xsl:if test="@interface[. = 'False'] and @abstract[. = 'True']">abstract </xsl:if>
<xsl:if test="@internal[. = 'True']">internal </xsl:if>
<xsl:if test="@enum[. = 'True']">enum </xsl:if>
<xsl:if test="@enum[. = 'False'] and @sealed[. ='True']">sealed </xsl:if>
<xsl:if test="@interface[. = 'True']">interface </xsl:if>
<xsl:if test="@class[. = 'True']">class </xsl:if>
<xsl:if test="@enum[. = 'False'] and @interface[. ='False'] and @class[. = 'False']">struct</xsl:if>
<xsl:if test="@const[. = 'True']">const </xsl:if>
<xsl:if test="@readonly[. = 'True']">readonly </xsl:if>
<xsl:if test="@virtual[. = 'True']">virtual </xsl:if>
<xsl:if test="@static[. = 'True']">static </xsl:if>
<xsl:if test="@final[. = 'True']">final </xsl:if>
</xsl:template>

<xsl:template match="BaseType">
Inherits from <xsl:value-of select="msnia:ApplyTypeLinks(.,$systemBaseUrl,$systemBaseUrlExt,$baseNs)" disable-output-escaping="yes"/><br/>
</xsl:template>

<xsl:template match="Type">
<xsl:value-of select="msnia:ApplyTypeLinks(.,$systemBaseUrl,$systemBaseUrlExt,$baseNs)" disable-output-escaping="yes"/> 
</xsl:template>

<xsl:template match="AsmName">
Defined in Assembly <xsl:value-of select="."/><br/>
</xsl:template>

<xsl:template match="Interfaces">
<xsl:if test="count(*) > 0">
<dl>
<dt>Implements interface<xsl:if test="count(*) > 1">s</xsl:if></dt>
	<xsl:apply-templates select="InterfaceType"/>
</dl>
</xsl:if>
</xsl:template>
<xsl:template match="InterfaceType">
	<dd><xsl:value-of select="msnia:ApplyTypeLinks(.,$systemBaseUrl,$systemBaseUrlExt,$baseNs)" disable-output-escaping="yes"/></dd>
</xsl:template>

<xsl:template match="img"><xsl:text disable-output-escaping="yes">&lt;img src="</xsl:text><xsl:value-of select="@src"/><xsl:text disable-output-escaping="yes">"&gt;</xsl:text></xsl:template>


</xsl:stylesheet>