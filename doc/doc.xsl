<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<!--To use, add
    <?xml-stylesheet type="text/xsl" href="../doc.xsl"?>
    To the top of any xml document, using the correct relative path.
-->
<xsl:template match="book">
	<html>
		<head>
			<title>BZFlag - <xsl:value-of select="bookinfo/title"/></title>
			<style type="text/css">
				a { font-size: 110%; text-decoration:none; display: block; color:#0000FF; background-color:#DAE2F6; }
				a:link.selected { display: block; color:#000000; background-color:#BED0EA; }
				a:visit.selected { display: block; color:#000000; background-color:#BED0EA; }
				a:hover.selected { display: block; color:#000000; background-color: #BED0EA; }
				a:link.unselected { display: block; color:#0000FF; background-color:#9FB5E0; }
				a:visit.unselected { display: block; color:#0000FF; background-color:#7090D0; }
				a:hover.unselected { display: block; color:#000000; background-color: #BED0EA; }
			</style>

			<script language="JavaScript">
				var curDiv = &quot;<xsl:value-of select="chapter[1]/@id"/>&quot;;
				function switchDiv( divName )
				{
					var el = document.getElementById( curDiv );
					el.style.display = "none";
					el = document.getElementById( curDiv + "_ctrl" );
					el.className = "unselected";
					el = document.getElementById( divName );
					el.style.display = "block";
					el = document.getElementById( divName + "_ctrl" );
					el.className = "selected";
					curDiv = divName;
				}
			</script>
		</head>
		<body bgcolor="#E0E0FF">
		<xsl:apply-templates select="bookinfo"/>
		<xsl:call-template name="ctrl"/>
		<xsl:call-template name="chapters"/>
		<script language="JavaScript">
			switchDiv(&quot;<xsl:value-of select="chapter[1]/@id"/>&quot;);
		</script>
		</body>
	</html>

</xsl:template>

<xsl:template match="bookinfo">
	<h1><xsl:value-of select="title"/></h1>
	<h4>published: <xsl:value-of select="pubdate"/></h4>
	<xsl:apply-templates select="abstract"/>
</xsl:template>

<xsl:template match="abstract">
	<hr/>
	<xsl:for-each select="para">
		<p><xsl:value-of select="."/></p>
	</xsl:for-each>
	<hr/>
</xsl:template>

<xsl:template name="ctrl">
	<xsl:variable name="chapWidth" select="100.0 div count(chapter)"/>
	<table border="1" width="100%">
		<tr>
		<xsl:for-each select="chapter">
			<xsl:element name="td">
				<xsl:attribute name="width"><xsl:value-of select="$chapWidth"/>%</xsl:attribute>
				<xsl:variable name="divid"><xsl:value-of select="@id"/></xsl:variable>
				<xsl:element name="a">
					<xsl:attribute name="href">javascript:switchDiv(&quot;<xsl:value-of select="$divid"/>&quot;)</xsl:attribute>
					<xsl:attribute name="class">unselected</xsl:attribute>
					<xsl:attribute name="id"><xsl:value-of select="@id"/>_ctrl</xsl:attribute>
					<xsl:value-of select="title"/>
				</xsl:element>
			</xsl:element>
		</xsl:for-each>
		</tr>
	</table>
</xsl:template>

<xsl:template name="chapters">
	<xsl:for-each select="chapter">
		<xsl:element name="div">
			<xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
			<xsl:attribute name="style">display:none;</xsl:attribute>
			<table width="100%">
				<xsl:for-each select="para|screen">
					<xsl:if test='name(.)="para"'>
						<tr><td>
							<xsl:value-of select="."/>
						</td></tr>
					</xsl:if>
					<xsl:if test='name(.)="screen"'>
						<tr><td><pre>
							<xsl:value-of select="."/>
						</pre></td></tr>
					</xsl:if>
				</xsl:for-each>
			</table>
		</xsl:element>
	</xsl:for-each>
</xsl:template>


</xsl:transform>
