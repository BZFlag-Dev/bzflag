<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<!--To use, add 
    <?xml-stylesheet type="text/xsl" href="../doc.xsl"?>
    To the top of any xml document, using the correct relative path.
    This is a work in progress the javascript calls are file related for some reason.
    More prettifying to do as well.
-->
<xsl:template match="book">
	<html>
		<head>
			<title>BZFlag - <xsl:value-of select="bookinfo/title"/></title>
			<script language="JavaScript">
				var curDiv = &quot;<xsl:value-of select="chapter[1]/@id"/>&quot;;
				function switchDiv( divName )
				{
					var el = document.getElementById( curDiv );
					el.style.display = "none";
					el = document.getElementById( divName );
					el.style.display = "block";
					curDiv = divName;
				}
			</script>
		</head>
		<body bgcolor="#E0E0FF">
		<xsl:apply-templates select="bookinfo"/>
		<xsl:call-template name="ctrl"/>
		<xsl:call-template name="chapters"/>
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
	<xsl:variable name="chapWidth" select="count(chapter)"/>
	<table border="1" width="100%">
		<tr>
		<xsl:for-each select="chapter">
			<xsl:element name="td">
				<xsl:attribute name="width"><xsl:value-of select="$chapWidth"/>%</xsl:attribute>
				<xsl:variable name="divid"><xsl:value-of select="@id"/></xsl:variable>
				<xsl:element name="a">
					<xsl:attribute name="href">&apos;javascript:switchDiv(&quot;<xsl:value-of select="$divid"/>&quot;)&apos;</xsl:attribute>
					<xsl:value-of select="title"/>
				</xsl:element>
			</xsl:element>
		</xsl:for-each>
		</tr>
	</table>
</xsl:template>

<xsl:template name="chapters">
	<table border = "0" width="100%">
		<xsl:for-each select="chapter">
			<div id="@id" style="display:none;">
				<xsl:for-each select="para">
					<tr><td>
						<xsl:value-of select="."/>
					</td></tr>
				</xsl:for-each>
			</div>
		</xsl:for-each>
	</table>
	<script language="JavaScript">
		switchDiv(&quot;<xsl:value-of select="chapter[1]/@id"/>&quot;);
	</script>
</xsl:template>


</xsl:transform>