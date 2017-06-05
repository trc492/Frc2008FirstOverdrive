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
string formatMethodSyntax(XPathNodeIterator method,string systemBaseUrl,string baseUrlExt,string baseNs){
string output = "";
string nameVal;
XPathNavigator xpn;
XmlNode xn;
XmlNode xp;
XmlAttribute attrib;
XmlNode assembly;
string[] param;
string paramlist;
	try{
		xpn = method.Current;
		xn = ((IHasXmlNode)xpn).GetNode();
		nameVal = xn.ParentNode.ParentNode.Attributes["name"].Value;
		assembly = xn.ParentNode.ParentNode.SelectSingleNode("Assembly");
		attrib = xn.Attributes["type"];

        string parent = xn.Attributes["name"].Value;
		
		if (parent.IndexOf("(") > -1)
			parent = parent.Substring(0,parent.IndexOf("("));

		if (assembly != null){
			output += MethodDecoration(assembly,systemBaseUrl,baseUrlExt,baseNs);
		}
		else
		{
	        if(attrib != null)
	        {
		        output += attrib.Value + " ";
	        }
		}


		if (nameVal.IndexOf("#ctor") > -1){
			//parse class name
			nameVal = xn.Attributes["name"].Value;
			nameVal = nameVal.Substring(0,nameVal.IndexOf(".#ctor"));
			nameVal = nameVal.Substring(nameVal.LastIndexOf(".") + 1);
		}
		output += nameVal + "(\n";

		paramlist = xn.Attributes["name"].Value;
		if (paramlist.IndexOf(",") > -1){
			paramlist = paramlist.Substring(paramlist.IndexOf("(") + 1);
			paramlist = paramlist.TrimEnd(')');
			
			param = paramlist.Split(',');
			for (int i = 0; i < param.Length; i++){
				xp = xn.SelectNodes("param")[i];
				param[i] = ApplyTypeLinks(param[i],systemBaseUrl,baseUrlExt,baseNs);
				if (xp != null){
					output += "\t" + param[i] +	xp.Attributes["name"].Value;
					if (i < param.Length - 1)
						output += ",";

					//output += ApplyParamFormatting(xp,xn,systemBaseUrl,baseUrlExt,baseNs);
					output += "\n";
				}else{
					output += "\t" + param[i] +
					" //<span class='missing'>param element not supplied</span>\n";
				}
			}
		}else{
			if (paramlist.IndexOf("(") > -1){
				
				paramlist = paramlist.Substring(paramlist.IndexOf("(") + 1);
				paramlist = paramlist.TrimEnd(')');
				paramlist = ApplyTypeLinks(paramlist,systemBaseUrl,baseUrlExt,baseNs);
				if(xn.SelectNodes("param").Count > 0){
					xp = xn.SelectNodes("param")[0]; //should only be one param
					if (xp != null){
						output += "\t" + paramlist + xp.Attributes["name"].Value;
			
						//output += ApplyParamFormatting(xp,xn,systemBaseUrl,baseUrlExt,baseNs);
						output += "\n";
					}
				}else{
				
					output += "\t" + paramlist + " //<span class='missing'>param element not supplied</span>\n";				
				}
			}
		}
		output += ");";

	}catch(Exception e){

		output = e.Message;
	}

return output;
}
string MethodDecoration(XmlNode assembly, string systemBaseUrl, string baseUrlExt,string baseNs){
	string ret = String.Empty;
	string returnType = String.Empty;
	
	if (assembly.Attributes["Error"] != null){
		return ret;
	}
	if (assembly.Attributes["return"] != null){
		returnType = assembly.Attributes["return"].InnerText;
	}

	if (IsAttributeTrue(assembly,"abstract") &&
		IsAttributeTrue(assembly,"virtual") &&
		IsAttributeTrue(assembly,"public")){
		//interface method
		if (returnType != String.Empty){
			if (returnType != "System.Void"){
				ret = ApplyTypeLinks(returnType,systemBaseUrl,baseUrlExt,baseNs);			
			}else{
				ret = "void ";
			}
		}
	}else{
		ret = "";
		if (IsAttributeTrue(assembly,"public")){
			ret += "public ";
		}else if (IsAttributeTrue(assembly,"protected")){
			ret += "protected ";
		}else if (IsAttributeTrue(assembly,"internal")){
			ret += "internal ";
		}else{
			ret += "private ";
		}
		
		if (IsAttributeTrue(assembly,"static")){
			ret += "static ";
		}
		if (IsAttributeTrue(assembly,"abstract")){
			ret += "abstract ";
		}
		if (!IsAttributeTrue(assembly,"final") &&
			IsAttributeTrue(assembly,"virtual")){
			ret += "virtual ";
		}
		if (returnType != String.Empty){
			if (returnType != "System.Void"){
				ret += ApplyTypeLinks(returnType, systemBaseUrl,baseUrlExt,baseNs);			
			}else{
				ret += "void ";
			}
		}
	}				
	
	return ret;

}

string ApplyTypeLinks(string type,string systemBaseUrl,string baseUrlExt,string baseNs){

	string output = "<a href=\"{0}\" class=\"typeLink\">{1}</a>";
	bool array = false;
	bool isSystem = false;
	
	if (baseNs == String.Empty){
		isSystem = type.IndexOf("System.") == 0;
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
string ApplyParamFormatting(XmlNode paramNode,XmlNode xn, string systemBaseUrl,string baseUrlExt, string baseNs){
string output = String.Empty;
bool isSystem = false;
string cref;
XmlElement xe;
XmlAttribute xa;
if (paramNode != null){
	if (paramNode.InnerText != String.Empty){
	
	
		output += " <span nowrap='true' class='comment'>//";
		if (paramNode.SelectNodes("see").Count > 0){
			foreach (XmlNode s in paramNode.SelectNodes("see")){
				cref = s.Attributes["cref"].Value.Substring(2);
				if (baseNs == String.Empty){
					isSystem = cref.IndexOf("System.") == 0;
				}else{
					isSystem = cref.IndexOf(baseNs) > -1;
				}
				xe = xn.OwnerDocument.CreateElement("a");
				xa = xn.OwnerDocument.CreateAttribute("href");
				if (cref.IndexOf("System.") > - 1 && !isSystem){
					xa.Value = systemBaseUrl + cref.Replace(".","") + "MembersTopic." + baseUrlExt;
				}else{
					xa.Value = cref + ".html";
								
				}					
				xe.Attributes.Append(xa);
				xe.InnerText = cref;					
				s.ParentNode.ReplaceChild(xe,s);
			}
		}
			
		output += paramNode.InnerText + "</span>";
	}
	output += "\n";
}
return output;

}
]]> </msxsl:script>


<xsl:template match="/">
<HTML>
<HEAD>
<TITLE>
<xsl:if test="Methods">	
	<xsl:choose>
		<xsl:when test="Methods/@name[ . ='#ctor']">Constructor</xsl:when>
	    <xsl:when test="substring( @id , 3, 10) = 'NULL.NULL.'">
		    <xsl:value-of select="Methods/@name"/> Function
		</xsl:when>
		<xsl:otherwise><xsl:value-of select="Methods/@name"/> Method </xsl:otherwise>
	</xsl:choose>
</xsl:if>
<xsl:if test="Method">	
	<xsl:choose>
		<xsl:when test="Method/@name[ . ='#ctor']">Constructor</xsl:when>
	    <xsl:when test="substring( Method/Docs/member/@name , 3, 10) = 'NULL.NULL.'">
		    <xsl:value-of select="Method/@name"/> Function
		</xsl:when>
		<xsl:otherwise><xsl:value-of select="Method/@name"/> Method </xsl:otherwise>
	</xsl:choose>
</xsl:if>
</TITLE>
<link rel="stylesheet" type="text/css"><xsl:attribute name="href"><xsl:value-of select="$cssFile"/></xsl:attribute></link>
</HEAD>
<BODY>
<dl>
	<dt><br/></dt>
	<dd>
	<table>
	<xsl:apply-templates select="Methods"/>
	<xsl:apply-templates select="Method"/>
	</table>
	</dd>
</dl>
</BODY>
</HTML>
</xsl:template>

<xsl:template match="Methods">
	<h1>
	<xsl:choose>
		<xsl:when test="@name[ . ='#ctor']">Constructor</xsl:when>
        <xsl:when test="substring( @id , 3, 10) = 'NULL.NULL.'">
		    <xsl:value-of select="@name"/> Function
		</xsl:when>
		<xsl:otherwise><xsl:value-of select="@name"/> Method </xsl:otherwise>
	</xsl:choose></h1>
	<xsl:apply-templates select="Method"/>
	
	<xsl:if test="substring( @id , 3, 10) != 'NULL.NULL.'">
	<tr>
		<td>Member of
		<a>
			<xsl:attribute name="href"><xsl:value-of select="substring(substring-before(@id,@name),3)"/>html</xsl:attribute>
            <xsl:value-of select="msnia:readableName(substring-before(@id,@name))" />
		</a>	
		</td>
	</tr>
	</xsl:if>
</xsl:template>
<xsl:template match="@name">
</xsl:template>
<xsl:template match="Method">
	<xsl:if test="name(parent::node()) != 'Methods'">
		<h1>
		<xsl:choose>
			<xsl:when test="@name[ . ='#ctor']">Constructor</xsl:when>
		    <xsl:when test="substring( Docs/member/@name , 3, 10) = 'NULL.NULL.'">
		        <xsl:value-of select="@name"/> Function
		    </xsl:when>
			<xsl:otherwise><xsl:value-of select="@name"/> Method </xsl:otherwise>
		</xsl:choose></h1>
	</xsl:if>
	<tr valign="top">
	<td>
	<xsl:apply-templates select="Docs/member"/>
	</td>
	</tr>

	<xsl:if test="name(parent::node()) != 'Methods'">	
	<xsl:if test="substring( Docs/member/@name , 3, 10) != 'NULL.NULL.'">
	<tr>
		<td>
		Member of
		<a>
			<xsl:attribute name="href"><xsl:value-of select="substring(substring-before(Docs/member/@name,@name),3)"/>html</xsl:attribute>
            <xsl:value-of select="msnia:readableName(substring-before(Docs/member/@name,@name))" />
		</a>
		</td>
	</tr>
	</xsl:if>
    </xsl:if>

	<xsl:if test="Docs/member/@source">
	    <tr>
	    <td>
        Source File: <xsl:value-of select="Docs/member/@source"/>
        </td>
	    </tr>
	</xsl:if>

</xsl:template>

<xsl:template match="member">
	<xsl:apply-templates select="summary"/>

	<dl>
	<dt><h4>Syntax</h4></dt>
	<dd>
		<pre class="syntax">
		<xsl:value-of select="msnia:formatMethodSyntax(.,$systemBaseUrl,$systemBaseUrlExt,$baseNs)" disable-output-escaping="yes"/>
		</pre>
	</dd>
	<xsl:if test="param">
		<dt><h4>Parameters</h4></dt>
		<dd><xsl:apply-templates select="param"/></dd>
	</xsl:if>

	<xsl:apply-templates select="returns"/>
	
	<xsl:if test="exception">
		<dt><h4>Exceptions</h4></dt>
		<dd><xsl:apply-templates select="exception"/></dd>
	</xsl:if>
	
	<xsl:apply-templates select="remarks"/>
	<xsl:apply-templates select="example"/>

	<xsl:if test="//seealso">
	<dt>
		<h4>See Also</h4>
	</dt>
	<dd>
		<xsl:apply-templates select="//seealso"/>
	</dd>
	</xsl:if>
	</dl>
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