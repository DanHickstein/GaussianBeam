<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- Convert GaussianBeam 1.0 files to 1.1 files -->

<xsl:output method="xml" indent="yes"/>

<!-- Default -->

<xsl:template match="*">
	<xsl:copy-of select="."/>
</xsl:template>

<!-- Fit -->

<xsl:template match="gaussianBeam[@version = '1.0']/waistFit">
	<beamFit id="0">
		<name>Fit</name>
		<dataType><xsl:value-of select="fitDataType"/></dataType>
		<xsl:variable name="firstData" select="position()"/>
		<xsl:for-each select="fitData">
			<xsl:element name="data">
			<xsl:attribute name="id"><xsl:value-of select="position() - $firstData"/></xsl:attribute>
				<position><xsl:value-of select="dataPosition"/></position>
				<value><xsl:value-of select="dataValue"/></value>
			</xsl:element>
		</xsl:for-each>
	</beamFit>
</xsl:template>

<!-- Target beam -->

<xsl:template match="gaussianBeam[@version = '1.0']/magicWaist">
	<targetBeam id="0">
		<position><xsl:value-of select="targetPosition"/></position>
		<waist><xsl:value-of select="targetWaist"/></waist>
		<xsl:apply-templates select="positionTolerance"/>
		<xsl:apply-templates select="waistTolerance"/>
	</targetBeam>
</xsl:template>

<!-- Optics -->

<xsl:template match = "gaussianBeam[@version = '1.0']/*/relativeLockParent">
	<relativeLockParent>
		<xsl:variable name="lockParent"><xsl:value-of select="."/></xsl:variable>
		<xsl:variable name="firstOptics" select="count(../../inputBeam/preceding-sibling::*)"/>
		<xsl:for-each select="../../*">
			<xsl:if test="name=$lockParent">
				<xsl:value-of select="count(./preceding-sibling::*) - $firstOptics + 1"/>
			</xsl:if>
		</xsl:for-each>
	</relativeLockParent>
</xsl:template>

<xsl:template match="gaussianBeam[@version = '1.0']/inputBeam |
                     gaussianBeam[@version = '1.0']/lens |
                     gaussianBeam[@version = '1.0']/flatInterface |
                     gaussianBeam[@version = '1.0']/curvedInterface |
                     gaussianBeam[@version = '1.0']/flatMirror |
                     gaussianBeam[@version = '1.0']/curvedMirror |
                     gaussianBeam[@version = '1.0']/genericABCD">
	<xsl:variable name="firstOptics" select="count(../inputBeam/preceding-sibling::*)"/>
	<xsl:copy>
	<xsl:attribute name="id"><xsl:value-of select="count(./preceding-sibling::*) - $firstOptics + 1"/></xsl:attribute>
		<xsl:apply-templates/>
		<xsl:if test="not(absoluteLock)">
			<absoluteLock>
			<xsl:choose>
				<xsl:when test="name()='inputBeam'">1</xsl:when>
				<xsl:otherwise>0</xsl:otherwise>
			</xsl:choose>
			</absoluteLock>
		</xsl:if>
	</xsl:copy>
</xsl:template>

<!-- Root -->

<xsl:template match="gaussianBeam[@version = '1.0']">
<gaussianBeam version="1.1">
	<bench id="0">
		<xsl:apply-templates select="wavelength"/>
		<xsl:variable name="horizontalRange" select="display/HRange/."/>
		<leftBoundary><xsl:value-of select="display/HOffset"/></leftBoundary>
		<rightBoundary><xsl:value-of select="display/HOffset + $horizontalRange"/></rightBoundary>
		<xsl:apply-templates select="magicWaist"/>
		<xsl:apply-templates select="waistFit"/>
		<opticsList>
			<xsl:apply-templates select="inputBeam|lens|flatInterface|curvedInterface|flatMirror|curvedMirror|genericABCD"/>
		</opticsList>
	</bench>
	<view id="0" bench="0">
		<horizontalRange><xsl:value-of select="display/HRange"/></horizontalRange>
		<verticalRange><xsl:value-of select="display/VRange div 1000."/></verticalRange>
		<origin><xsl:value-of select="display/HOffset"/></origin>
		<showTargetBeam id="0"><xsl:value-of select="magicWaist/showTargetWaist"/></showTargetBeam>
	</view>
</gaussianBeam>
</xsl:template>

</xsl:stylesheet>
