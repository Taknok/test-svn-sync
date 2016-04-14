# -------------------------------------------------
# Project created by QtCreator 2009-02-14T15:30:46
# -------------------------------------------------

message(qmake version: $$[QMAKE_VERSION])
message(Qt version: $$[QT_VERSION])

#Qt5.4 required for QOpenGLWidget instead of QGLWidget
lessThan(QT_MAJOR_VERSION, 5) {
  error("Qt5.4 or greater is required for xflr5 v6.13")
}
else
{
    lessThan(QT_MINOR_VERSION, 4) {
	  error("Qt5.4 or greater is required for xflr5 v6.13")
    }
}


CONFIG += qt
QT += opengl
TEMPLATE = app

INCLUDEPATH += $$PWD/src
INCLUDEPATH += $$PWD/src/viewwidgets
INCLUDEPATH += $$PWD/src/objects
INCLUDEPATH += $$PWD/src/graph
INCLUDEPATH += $$PWD/src/misc
INCLUDEPATH += $$PWD/src/glcontextinfo

SOURCES += \
	src/XFLR5Application.cpp \
	src/globals.cpp \
	src/main.cpp \
	src/mainframe.cpp \
	src/viewwidgets/section2dwidget.cpp \
	src/viewwidgets/graphwidget.cpp \
	src/viewwidgets/graphtilewidget.cpp \
	src/viewwidgets/legendwidget.cpp \
	src/viewwidgets/gl3widget.cpp \
	src/misc/Settings.cpp \
	src/misc/SaveOptionsDlg.cpp \
	src/misc/ProgressDlg.cpp \
	src/misc/ModDlg.cpp \
	src/misc/PolarFilterDlg.cpp \
	src/misc/TranslatorDlg.cpp \
	src/misc/RenameDlg.cpp \
	src/misc/LinePickerDlg.cpp \
	src/misc/LineDelegate.cpp \
	src/misc/LineCbBox.cpp \
	src/misc/FloatEditDelegate.cpp \
	src/misc/ColorButton.cpp \
	src/misc/DoubleEdit.cpp \
	src/misc/IntEdit.cpp \
	src/misc/AboutQ5.cpp \
	src/misc/NewNameDlg.cpp \
	src/misc/ObjectPropsDlg.cpp \
	src/misc/W3dPrefsDlg.cpp \
	src/misc/LineBtn.cpp \
	src/misc/TextClrBtn.cpp \
	src/misc/MinTextEdit.cpp \
	src/misc/EditPlrDlg.cpp \
	src/misc/Units.cpp \
	src/misc/LengthUnitDlg.cpp \
	src/misc/exponentialslider.cpp \
	src/miarex/Miarex.cpp \
	src/miarex/Objects3D.cpp \
	src/miarex/analysis/StabPolarDlg.cpp \
	src/miarex/analysis/CtrlTableDelegate.cpp \
	src/miarex/analysis/WAdvancedDlg.cpp \
	src/miarex/analysis/WPolarDlg.cpp  \
	src/miarex/analysis/LLTAnalysisDlg.cpp \
	src/miarex/analysis/LLTAnalysis.cpp \
	src/miarex/analysis/PanelAnalysisDlg.cpp \
	src/miarex/analysis/PanelAnalysis.cpp \
	src/miarex/analysis/AeroDataDlg.cpp \
	src/miarex/analysis/EditPolarDefDlg.cpp \
	src/miarex/design/GL3dWingDlg.cpp \
	src/miarex/design/GL3dBodyDlg.cpp \
	src/miarex/design/PlaneDlg.cpp \
	src/miarex/design/WingDelegate.cpp \
	src/miarex/design/WingScaleDlg.cpp \
	src/miarex/design/InertiaDlg.cpp \
	src/miarex/design/BodyGridDlg.cpp \
	src/miarex/design/BodyScaleDlg.cpp \
	src/miarex/design/BodyTableDelegate.cpp \
	src/miarex/design/BodyTransDlg.cpp \
	src/miarex/design/EditObjectDelegate.cpp \
	src/miarex/design/EditPlaneDlg.cpp \
	src/miarex/design/EditBodyDlg.cpp \
	src/miarex/design/wingseldlg.cpp \
	src/miarex/view/TargetCurveDlg.cpp \
	src/miarex/view/GL3DScales.cpp \
	src/miarex/view/StabViewDlg.cpp \
	src/miarex/mgt/ImportObjectDlg.cpp\
	src/miarex/view/GLLightDlg.cpp \
	src/miarex/mgt/ManagePlanesDlg.cpp \
	src/miarex/mgt/PlaneTableDelegate.cpp \
	src/miarex/mgt/XmlPlaneReader.cpp \
	src/miarex/mgt/XmlPlaneWriter.cpp \
	src/xdirect/XDirect.cpp \
	src/xdirect/ManageFoilsDlg.cpp \
	src/xdirect/XDirectStyleDlg.cpp \
	src/xdirect/analysis/BatchDlg.cpp \
	src/xdirect/analysis/BatchThreadDlg.cpp \
	src/xdirect/analysis/FoilPolarDlg.cpp \
	src/xdirect/analysis/ReListDlg.cpp \
	src/xdirect/analysis/XFoil.cpp \
	src/xdirect/analysis/XFoilAdvancedDlg.cpp \
	src/xdirect/analysis/XFoilAnalysisDlg.cpp \
	src/xdirect/analysis/XFoilTask.cpp \
	src/xdirect/geometry/CAddDlg.cpp \
	src/xdirect/geometry/FlapDlg.cpp \
	src/xdirect/geometry/FoilCoordDlg.cpp \
	src/xdirect/geometry/FoilGeomDlg.cpp \
	src/xdirect/geometry/InterpolateFoilsDlg.cpp \
	src/xdirect/geometry/LEDlg.cpp \
	src/xdirect/geometry/NacaFoilDlg.cpp \
	src/xdirect/geometry/TEGapDlg.cpp \
	src/xdirect/geometry/TwoDPanelDlg.cpp \
	src/objects/ArcBall.cpp \
	src/objects/Body.cpp \
	src/objects/CVector.cpp \
	src/objects/Foil.cpp \
	src/objects/Frame.cpp \
	src/objects/OpPoint.cpp \
	src/objects/Panel.cpp \
	src/objects/Plane.cpp \
	src/objects/Polar.cpp \
	src/objects/Quaternion.cpp \
	src/objects/PlaneOpp.cpp \
	src/objects/NURBSSurface.cpp \
	src/objects/Spline.cpp \
	src/objects/Surface.cpp \
	src/objects/WPolar.cpp \
	src/objects/Wing.cpp \
	src/objects/SplineFoil.cpp \
	src/objects/WingOpp.cpp \
	src/graph/QGraph.cpp \
	src/graph/GraphDlg.cpp \
	src/graph/Graph.cpp \
	src/graph/Curve.cpp \
	src/xinverse/FoilSelectionDlg.cpp \
	src/xinverse/PertDlg.cpp \
	src/xinverse/XInverse.cpp \
	src/xinverse/InverseOptionsDlg.cpp \
	src/design/FoilTableDelegate.cpp \
	src/design/LECircleDlg.cpp \
	src/design/AFoil.cpp \
	src/design/SplineCtrlsDlg.cpp \
	src/design/AFoilTableDlg.cpp \
	src/design/GridSettingsDlg.cpp \
	src/viewwidgets/BodyFrameWidget.cpp \
	src/viewwidgets/BodyLineWidget.cpp \
	src/viewwidgets/Direct2dDesign.cpp \
	src/viewwidgets/miarextilewidget.cpp \
	src/viewwidgets/xdirecttilewidget.cpp \
	src/viewwidgets/wingwidget.cpp \
	src/viewwidgets/oppointwidget.cpp \
	src/viewwidgets/inverseviewwidget.cpp \
	src/glcontextinfo/glrenderwindow.cpp \
	src/glcontextinfo/openglinfodlg.cpp \
    src/misc/stlexportdialog.cpp \
    src/miarex/analysis/planeanalysistask.cpp


HEADERS += \
	src/params.h \
	src/xflr5.h \
	src/globals.h \
	src/mainframe.h \
	src/XFLR5Application.h \
	src/viewwidgets/section2dwidget.h \
	src/viewwidgets/graphwidget.h \
	src/viewwidgets/graphtilewidget.h \
	src/viewwidgets/legendwidget.h \
	src/viewwidgets/gl3widget.h \
	src/misc/Settings.h \
	src/misc/SaveOptionsDlg.h \
	src/misc/ModDlg.h \
	src/misc/PolarFilterDlg.h \
	src/misc/TranslatorDlg.h \
	src/misc/RenameDlg.h \
	src/misc/LinePickerDlg.h \
	src/misc/LineDelegate.h \
	src/misc/FloatEditDelegate.h \
	src/misc/W3dPrefsDlg.h \
	src/misc/ColorButton.h \
	src/misc/LineCbBox.h \
	src/misc/AboutQ5.h \
	src/misc/DoubleEdit.h \
	src/misc/IntEdit.h \
	src/misc/ProgressDlg.h \
	src/misc/NewNameDlg.h \
	src/misc/ObjectPropsDlg.h \
	src/misc/LineBtn.h \
	src/misc/TextClrBtn.h \
	src/misc/MinTextEdit.h \
	src/misc/EditPlrDlg.h \
	src/misc/Units.h \
	src/misc/LengthUnitDlg.h \
	src/misc/exponentialslider.h \
	src/miarex/Miarex.h \
	src/miarex/Objects3D.h \
	src/miarex/analysis/WAdvancedDlg.h \
	src/miarex/analysis/WPolarDlg.h \
	src/miarex/analysis/StabPolarDlg.h \
	src/miarex/analysis/CtrlTableDelegate.h \
	src/miarex/analysis/LLTAnalysisDlg.h \
	src/miarex/analysis/LLTAnalysis.h \
	src/miarex/analysis/PanelAnalysis.h \
	src/miarex/analysis/PanelAnalysisDlg.h \
	src/miarex/analysis/AeroDataDlg.h \
	src/miarex/analysis/EditPolarDefDlg.h \
	src/miarex/design/InertiaDlg.h \
	src/miarex/design/GL3dBodyDlg.h \
	src/miarex/design/WingScaleDlg.h \
	src/miarex/design/WingDelegate.h \
	src/miarex/design/PlaneDlg.h \
	src/miarex/design/BodyGridDlg.h \
	src/miarex/design/BodyTableDelegate.h \
	src/miarex/design/BodyScaleDlg.h \
	src/miarex/design/GL3dWingDlg.h \
	src/miarex/design/BodyTransDlg.h \
	src/miarex/design/EditObjectDelegate.h \
	src/miarex/design/EditPlaneDlg.h \
	src/miarex/design/EditBodyDlg.h \
	src/miarex/design/wingseldlg.h \
	src/miarex/view/TargetCurveDlg.h \
	src/miarex/view/GL3DScales.h \
	src/miarex/view/StabViewDlg.h \
	src/miarex/view/GLLightDlg.h \
	src/miarex/mgt/ImportObjectDlg.h \
	src/miarex/mgt/ManagePlanesDlg.h \
	src/miarex/mgt/PlaneTableDelegate.h \
	src/miarex/mgt/XmlPlaneReader.h \
	src/miarex/mgt/XmlPlaneWriter.h \
	src/xdirect/XDirect.h \
	src/xdirect/ManageFoilsDlg.h \
	src/xdirect/XDirectStyleDlg.h \
	src/xdirect/analysis/BatchDlg.h \
	src/xdirect/analysis/BatchThreadDlg.h \
	src/xdirect/analysis/FoilPolarDlg.h \
	src/xdirect/analysis/ReListDlg.h \
	src/xdirect/analysis/XFoil.h \
	src/xdirect/analysis/XFoilAdvancedDlg.h \
	src/xdirect/analysis/XFoilAnalysisDlg.h \
	src/xdirect/analysis/XFoilTask.h \
	src/xdirect/geometry/CAddDlg.h \
	src/xdirect/geometry/FlapDlg.h \
	src/xdirect/geometry/FoilCoordDlg.h \
	src/xdirect/geometry/FoilGeomDlg.h \
	src/xdirect/geometry/InterpolateFoilsDlg.h \
	src/xdirect/geometry/LEDlg.h \
	src/xdirect/geometry/NacaFoilDlg.h \
	src/xdirect/geometry/TEGapDlg.h \
	src/xdirect/geometry/TwoDPanelDlg.h \
	src/xinverse/XInverse.h \
	src/xinverse/InverseOptionsDlg.h \
	src/xinverse/FoilSelectionDlg.h \
	src/xinverse/PertDlg.h \
	src/objects/WPolar.h \
	src/objects/Wing.h \
	src/objects/Surface.h \
	src/objects/Spline.h \
	src/objects/WingSection.h \
	src/objects/OpPoint.h \
	src/objects/Quaternion.h \
	src/objects/PlaneOpp.h \
	src/objects/Polar.h \
	src/objects/CVector.h \
	src/objects/Plane.h \
	src/objects/CRectangle.h \
	src/objects/NURBSSurface.h \
	src/objects/Panel.h \
	src/objects/Frame.h \
	src/objects/Foil.h \
	src/objects/Body.h \
	src/objects/ArcBall.h \
	src/objects/SplineFoil.h \
	src/objects/WingOpp.h \
	src/objects/PointMass.h \
	src/objects/linestyle.h \
	src/graph/Graph.h \
	src/graph/GraphDlg.h \
	src/graph/Curve.h \
	src/graph/QGraph.h \
	src/design/AFoil.h \
	src/design/LECircleDlg.h \
	src/design/SplineCtrlsDlg.h \
	src/design/FoilTableDelegate.h \
	src/design/AFoilTableDlg.h \
	src/design/GridSettingsDlg.h \
	src/viewwidgets/BodyFrameWidget.h \
	src/viewwidgets/BodyLineWidget.h \
	src/viewwidgets/Direct2dDesign.h \
	src/viewwidgets/miarextilewidget.h \
	src/viewwidgets/xdirecttilewidget.h \
	src/viewwidgets/wingwidget.h \
	src/viewwidgets/oppointwidget.h \
	src/viewwidgets/inverseviewwidget.h \
	src/glcontextinfo/glrenderwindow.h \
	src/glcontextinfo/openglinfodlg.h \
    src/misc/stlexportdialog.h \
    src/miarex/analysis/planeanalysistask.h


TRANSLATIONS = translations/xflr5v6.ts \
    translations/xflr5v6_de.ts \
    translations/xflr5v6_fr.ts \
    translations/xflr5v6_ja.ts

RESOURCES += \
	images.qrc \
	shaders.qrc \
	textures.qrc

win32 {
    TARGET = XFLR5
    RC_FILE = win/xflr5.rc
	LIBS += -lopenGL32
#QMAKE_LFLAGS_WINDOWS += -Wl,--large-address-aware
}

unix{
    TARGET = xflr5
    
#	release: DESTDIR = ../build/release
#	debug:   DESTDIR = ../build/debug

#	OBJECTS_DIR = $$DESTDIR/.o
#	MOC_DIR = $$DESTDIR/.moc
#	RCC_DIR = $$DESTDIR/.rcc

	# VARIABLES
	isEmpty(PREFIX):PREFIX = /usr
	BINDIR = $$PREFIX/bin
	DATADIR = $$PREFIX/share

	# MAKE INSTALL
	INSTALLS += target
	target.path = $$BINDIR
}

macx { 
    CONFIG(release, debug|release) { 
        OBJECTS_DIR = ./build/release
        MOC_DIR = ./build/release
        RCC_DIR = ./build/release
        UI_HEADERS_DIR = ./build/release
    }
    CONFIG(debug, debug|release) { 
        OBJECTS_DIR = ./build/debug
        MOC_DIR = ./build/debug
		RCC_DIR = ./build/debug
        UI_HEADERS_DIR = ./build/debug
    }
    TARGET = XFLR5
    TEMPLATE = app
    CONFIG += i386
    QMAKE_MAC_SDK = macosx10.11
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
    OTHER_FILES += mac/Info.plist
    LIBS += -framework \
        CoreFoundation
    QMAKE_INFO_PLIST = mac/Info.plist
    ICON = mac/xflr5.icns
}
OTHER_FILES += doc/ReleaseNotes.txt \
    qss/appwidget.css \
    qss/xflr5_style.qss \
    qss/default.qss


QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4


#MAKE_LFLAGS_WINDOWS += Wl, -heap,500000000

#QMAKE_LFLAGS_WINDOWS += Wl, -heap, 500000000

#profiling flags
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

#QMAKE_CXXFLAGS += -Wall

DISTFILES += \
    images/wing_left_bottom.png \
    images/wing_right_bottom.png \
    images/wing_right_top.png

