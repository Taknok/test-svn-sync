#ifndef XFLR5_H
#define XFLR5_H

// first name space
namespace XFLR5
{
	/** @enum The different types of analysis methods for 3D calculations of wings and planes. */
	typedef enum {LLTMETHOD, VLMMETHOD, PANELMETHOD} enumAnalysisMethod;

	/** @enum The different types of polar available for 2D and 3D calculations. */
	typedef enum {FIXEDSPEEDPOLAR, FIXEDLIFTPOLAR, RUBBERCHORDPOLAR, FIXEDAOAPOLAR, STABILITYPOLAR, BETAPOLAR} enumPolarType;

	/** @enum The different applications available to the user */
	typedef enum {NOAPP, XFOILANALYSIS, DIRECTDESIGN, INVERSEDESIGN, MIAREX, OPENGL3}	enumApp;

	/**< @enum The different image formats usable to export screen captures*/
	typedef enum {PNG, JPEG, BMP}	enumImageFormat;

	/** @enum The different formats usable to export data to text format files*/
	typedef enum {TXT, CSV}	enumTextFileType;

	/** @enum The different number of graphs in the polar view */
	typedef enum {ONEGRAPH, TWOGRAPHS, FOURGRAPHS, ALLGRAPHS} enumGraphView;

	/** @enum The different type of reference areaand length */
	typedef enum {PLANFORMREFDIM, PROJECTEDREFDIM, MANUALREFDIM} enumRefDimension;

	/** This enumeration defines the options for the active view.
	 *May be the operaring point view, the polar view, the 3D view, the Cp view, or the stability view*/
	typedef enum {WOPPVIEW, WPOLARVIEW,  WCPVIEW, W3DVIEW, STABTIMEVIEW, STABPOLARVIEW, OTHERVIEW} enumMiarexViews;

	/** @enum The two types of body : flat panels or NURBS*/
	typedef enum  {BODYPANELTYPE, BODYSPLINETYPE }     enumBodyLineType;

	/** @enum The 4 types of wing */
	typedef enum {MAINWING, SECONDWING, ELEVATOR, FIN, OTHERWING} enumWingType;

	/** @enum The different types of panel distribution on the wing */
	typedef enum {COSINE, UNIFORM, SINE, INVERSESINE} enumPanelDistribution;

	/** @enum The two types of boundary conditions for 3D panel analysis. VLM BC are of the NEUMANN type*/
	typedef enum {DIRICHLET, NEUMANN} enumBC;

	typedef enum {BOOL, INTEGER, DOUBLE, STRING, PANELDISTRIBUTION, FOILNAME, BODYTYPE, POLARTYPE, ANALYSISMETHOD, REFDIMENSIONS, WINGTYPE, BOUNDARYCONDITION} enumDataType;

}

#endif // XFLR5_H

