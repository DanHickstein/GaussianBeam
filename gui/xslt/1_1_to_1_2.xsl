<?xml version="1.0"?>
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- Convert GaussianBeam 1.1 files to 1.2 files -->

<xsl:output method="xml" indent="yes"/>

<!-- Default -->

<xsl:template match="@*|node()">
	<xsl:copy>
		<xsl:apply-templates select="@*|node()"/>
	</xsl:copy>
</xsl:template>

<!-- TODO: flip mirrors, viewverticalRange -->

<!-- targetBeam -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/targetBeam">
	<xsl:copy>
		<xsl:apply-templates select="@*"/>
		<waist orientation="0"><xsl:value-of select="waist"/></waist>
		<waistPosition orientation="0"><xsl:value-of select="position"/></waistPosition>
		<wavelength><xsl:value-of select="../wavelength"/></wavelength>
		<index>1</index>
		<M2>1</M2>
		<targetOverlap><xsl:value-of select="minOverlap"/></targetOverlap>
		<targetOrientation>0</targetOrientation>
	</xsl:copy>
</xsl:template>

<!-- beamFit -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/beamFit">
	<xsl:copy>
		<xsl:apply-templates select="@*|name|dataType|color"/>
		<orientation>0</orientation>
		<xsl:apply-templates select="data"/>
	</xsl:copy>
</xsl:template>

<!-- beamFit value -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/beamFit/data/value">
	<value orientation="0"><xsl:value-of select="."/></value>
</xsl:template>

<!-- inputBeam -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/opticsList/inputBeam">
<createBeam>
	<xsl:apply-templates select="@id"/>
	<beam>
		<waist orientation="0"><xsl:value-of select="waist"/></waist>
		<waistPosition orientation="0"><xsl:value-of select="position"/></waistPosition>
		<wavelength><xsl:value-of select="../../wavelength"/></wavelength>
		<xsl:apply-templates select="index|M2"/>
	</beam>
	<position>0.</position>
	<xsl:apply-templates select="name|absoluteLock|relativeLockParent"/>
</createBeam>
</xsl:template>

<!-- flatMirror -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/opticsList/flatMirror">
<flatMirror>
	<xsl:apply-templates select="@id|position|name|absoluteLock|relativeLockParent"/>
	<angle>3.141592653</angle>
</flatMirror>
</xsl:template>

<!-- curvedMirror -->

<xsl:template match="gaussianBeam[@version = '1.1']/bench/opticsList/curvedMirror">
<curvedMirror>
	<xsl:apply-templates select="@id|position|curvatureRadius|name|absoluteLock|relativeLockParent"/>
	<angle>3.141592653</angle>
</curvedMirror>
</xsl:template>

<!-- verticalRange TODO -->
<xsl:template match="gaussianBeam[@version = '1.1']/view/verticalRange">
</xsl:template>

<!-- Root -->

<xsl:template match="gaussianBeam[@version = '1.1']">
<gaussianBeam version="1.2">
	<xsl:apply-templates/>
</gaussianBeam>
</xsl:template>

</xsl:stylesheet>
