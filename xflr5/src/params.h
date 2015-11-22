

/**
*@file
*
* This files defines the values of the main constant parameters used throughout the program.
*
* A modification of this file triggers the compilation of the whole project.
* 
*/
#ifndef PARAMS_H
#define PARAMS_H


#define VERSIONNAME     "xflr5 v6.12"

//General
#define PI             3.14159265358979  
#define MAXRECENTFILES         8  /**< Defines the maximum number of file names in the recent file list */
#define SETTINGSFORMAT     53753  /**< A random number which defines the format of the settings file */
#define PRECISION  0.00000000001  /**< Values are assumed 0 if less than this value. This is to avoid comparing the equality of two floating point numbers */




//XFoil Direct Parameters - refer to XFoil documentation
#define IQX  302	/**< 300 = number of surface panel nodes + 6 */
#define IQX2 151	/**< IQX/2 */
#define IWX   50	/**< number of wake panel nodes */
#define IPX    6	/**< 6 number of qspec[s] distributions */
#define ISX    3	/**< number of airfoil sides */
#define IBX  604	/**< 600 number of buffer airfoil nodes = 2*IQX */
#define IZX  350	/**< 350 = number of panel nodes [airfoil + wake] */
#define IVX  302	/**< 300 = number of nodes along bl on one side of airfoil and wake. */

//XFoil INVERSE parameters  - refer to XFoil documentation
#define ICX 257     /**< number of circle-plane points for complex mapping   ( 2^n  + 1 ) */
#define IMX 64      /**< number of complex mapping coefficients  Cn */
#define IMX4 16     /**< = IMX/4 */


//3D analysis parameters
#define MAXWINGS            4     /**< Wing, wing2, elevator, fin, in that order.*/
#define MAXBODIES           1     /**< One only in XFLR5 */
#define MAXSPANSTATIONS   250     /**< The max number of stations for LLT. For a VLM analysis, this is the max number of panels in the spanwise direction. */


#define MIDPOINTCOUNT 100  /**< the number of points which define the foil's mid camber line */





#define QUESTION (BB || !BB) /**< Shakespeare */

#endif // PARAMS_H
 
