<?xml version="1.0" encoding="utf-8"?> 
<xsl:stylesheet 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"> 

	<xsl:import 
    	href="/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl"/> 
	<xsl:output method="html" 
		encoding="gb2312" 
		indent="yes" />
	<xsl:template name="user.head.content">
		<xsl:comment>
    #######################################################################
    # THIS HTML FILE IS GENERATED FROM XML SOURCE. DO NOT EDIT THIS FILE! #
    #######################################################################
</xsl:comment>
	</xsl:template>

	<xsl:param name="html.stylesheet" select="'wForum.css'"/>
	<xsl:param name="section.autolabel" select="1"/>
	<xsl:param name="toc.section.depth" select="3"/>
	<xsl:param name="ulink.target" select="'_blank'"></xsl:param>
</xsl:stylesheet> 