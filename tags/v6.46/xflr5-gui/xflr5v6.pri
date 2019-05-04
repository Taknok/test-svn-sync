
SOURCES += \
    design/AFoil.cpp \
    design/AFoilTableDlg.cpp \
    design/FoilTableDelegate.cpp \
    design/GridSettingsDlg.cpp \
    design/LECircleDlg.cpp \
    design/SplineCtrlsDlg.cpp \
    glcontextinfo/glrenderwindow.cpp \
    glcontextinfo/view3dtestdlg.cpp \
    globals/XFLR5Application.cpp \
    globals/globals.cpp \
    globals/main.cpp \
    globals/mainframe.cpp \
    graph/curve.cpp \
    graph/graph.cpp \
    graph/graph_globals.cpp \
    graph/graphdlg.cpp \
    gui_objects/Spline5.cpp \
    gui_objects/SplineFoil.cpp \
    miarex/Miarex.cpp \
    miarex/analysis/AeroDataDlg.cpp \
    miarex/analysis/CtrlTableDelegate.cpp \
    miarex/analysis/EditPolarDefDlg.cpp \
    miarex/analysis/LLTAnalysisDlg.cpp \
    miarex/analysis/PanelAnalysisDlg.cpp \
    miarex/analysis/StabPolarDlg.cpp \
    miarex/analysis/WAdvancedDlg.cpp \
    miarex/analysis/WPolarDlg.cpp  \
    miarex/design/BodyGridDlg.cpp \
    miarex/design/BodyScaleDlg.cpp \
    miarex/design/BodyTableDelegate.cpp \
    miarex/design/BodyTransDlg.cpp \
    miarex/design/EditBodyDlg.cpp \
    miarex/design/EditObjectDelegate.cpp \
    miarex/design/EditPlaneDlg.cpp \
    miarex/design/GL3dBodyDlg.cpp \
    miarex/design/GL3dWingDlg.cpp \
    miarex/design/InertiaDlg.cpp \
    miarex/design/PlaneDlg.cpp \
    miarex/design/WingDelegate.cpp \
    miarex/design/WingScaleDlg.cpp \
    miarex/design/wingseldlg.cpp \
    miarex/mgt/ImportObjectDlg.cpp\
    miarex/mgt/ManagePlanesDlg.cpp \
    miarex/mgt/PlaneTableDelegate.cpp \
    miarex/mgt/XmlPlaneReader.cpp \
    miarex/mgt/XmlPlaneWriter.cpp \
    miarex/mgt/xmlwpolarreader.cpp \
    miarex/mgt/xmlwpolarwriter.cpp \
    miarex/objects3d.cpp \
    miarex/view/GL3DScales.cpp \
    miarex/view/GLLightDlg.cpp \
    miarex/view/StabViewDlg.cpp \
    miarex/view/TargetCurveDlg.cpp \
    miarex/view/W3dPrefsDlg.cpp \
    misc/AboutQ5.cpp \
    misc/EditPlrDlg.cpp \
    misc/LengthUnitDlg.cpp \
    misc/ModDlg.cpp \
    misc/NewNameDlg.cpp \
    misc/ObjectPropsDlg.cpp \
    misc/PolarFilterDlg.cpp \
    misc/ProgressDlg.cpp \
    misc/RenameDlg.cpp \
    misc/color/ColorButton.cpp \
    misc/exponentialslider.cpp \
    misc/line/LineBtn.cpp \
    misc/line/LineCbBox.cpp \
    misc/line/LineDelegate.cpp \
    misc/line/LinePickerDlg.cpp \
    misc/options/displayoptions.cpp \
    misc/options/languagewt.cpp \
    misc/options/preferencesdlg.cpp \
    misc/options/saveoptions.cpp \
    misc/options/units.cpp \
    misc/popup.cpp \
    misc/stlexportdialog.cpp \
    misc/text/DoubleEdit.cpp \
    misc/text/FloatEditDelegate.cpp \
    misc/text/IntEdit.cpp \
    misc/text/MinTextEdit.cpp \
    misc/text/TextClrBtn.cpp \
    misc/updater.cpp \
    misc/voidwidget.cpp \
    misc/waitdlg.cpp \
    script/xflscriptexec.cpp \
    script/xflscriptreader.cpp \
    viewwidgets/glWidgets/ArcBall.cpp \
    viewwidgets/glWidgets/gl3dbodyview.cpp \
    viewwidgets/glWidgets/gl3dmiarexview.cpp \
    viewwidgets/glWidgets/gl3dplaneview.cpp \
    viewwidgets/glWidgets/gl3dtestview.cpp \
    viewwidgets/glWidgets/gl3dview.cpp \
    viewwidgets/glWidgets/gl3dwingview.cpp \
    viewwidgets/graphtilewidget.cpp \
    viewwidgets/graphwidget.cpp \
    viewwidgets/inverseviewwidget.cpp \
    viewwidgets/legendwidget.cpp \
    viewwidgets/miarextilewidget.cpp \
    viewwidgets/oppointwidget.cpp \
    viewwidgets/section2dwidget.cpp \
    viewwidgets/wingwidget.cpp \
    viewwidgets/xdirecttilewidget.cpp \
    xdirect/ManageFoilsDlg.cpp \
    xdirect/XDirect.cpp \
    xdirect/XDirectStyleDlg.cpp \
    xdirect/analysis/BatchDlg.cpp \
    xdirect/analysis/BatchThreadDlg.cpp \
    xdirect/analysis/FoilPolarDlg.cpp \
    xdirect/analysis/ReListDlg.cpp \
    xdirect/analysis/XFoilAdvancedDlg.cpp \
    xdirect/analysis/XFoilAnalysisDlg.cpp \
    xdirect/analysis/XFoilTask.cpp \
    xdirect/geometry/CAddDlg.cpp \
    xdirect/geometry/FlapDlg.cpp \
    xdirect/geometry/FoilCoordDlg.cpp \
    xdirect/geometry/FoilGeomDlg.cpp \
    xdirect/geometry/InterpolateFoilsDlg.cpp \
    xdirect/geometry/LEDlg.cpp \
    xdirect/geometry/NacaFoilDlg.cpp \
    xdirect/geometry/TEGapDlg.cpp \
    xdirect/geometry/TwoDPanelDlg.cpp \
    xdirect/objects2d.cpp \
    xdirect/xmlpolarreader.cpp \
    xdirect/xmlpolarwriter.cpp \
    xinverse/FoilSelectionDlg.cpp \
    xinverse/InverseOptionsDlg.cpp \
    xinverse/PertDlg.cpp \
    xinverse/XInverse.cpp \
    $$PWD/viewwidgets/foildesignwt.cpp \
    $$PWD/viewwidgets/bodylinewt.cpp \
    $$PWD/viewwidgets/bodyframewt.cpp


HEADERS += \
    design/AFoil.h \
    design/AFoilTableDlg.h \
    design/FoilTableDelegate.h \
    design/GridSettingsDlg.h \
    design/LECircleDlg.h \
    design/SplineCtrlsDlg.h \
    glcontextinfo/glrenderwindow.h \
    glcontextinfo/view3dtestdlg.h \
    globals/XFLR5Application.h \
    globals/globals.h \
    globals/gui_enums.h \
    globals/gui_params.h \
    globals/mainframe.h \
    graph/curve.h \
    graph/graph.h \
    graph/graph_globals.h \
    graph/graphdlg.h \
    graph/linestyle.h \
    gui_objects/Spline5.h \
    gui_objects/SplineFoil.h \
    miarex/Miarex.h \
    miarex/analysis/AeroDataDlg.h \
    miarex/analysis/CtrlTableDelegate.h \
    miarex/analysis/EditPolarDefDlg.h \
    miarex/analysis/LLTAnalysisDlg.h \
    miarex/analysis/PanelAnalysisDlg.h \
    miarex/analysis/StabPolarDlg.h \
    miarex/analysis/WAdvancedDlg.h \
    miarex/analysis/WPolarDlg.h \
    miarex/design/BodyGridDlg.h \
    miarex/design/BodyScaleDlg.h \
    miarex/design/BodyTableDelegate.h \
    miarex/design/BodyTransDlg.h \
    miarex/design/EditBodyDlg.h \
    miarex/design/EditObjectDelegate.h \
    miarex/design/EditPlaneDlg.h \
    miarex/design/GL3dBodyDlg.h \
    miarex/design/GL3dWingDlg.h \
    miarex/design/InertiaDlg.h \
    miarex/design/PlaneDlg.h \
    miarex/design/WingDelegate.h \
    miarex/design/WingScaleDlg.h \
    miarex/design/wingseldlg.h \
    miarex/mgt/ImportObjectDlg.h \
    miarex/mgt/ManagePlanesDlg.h \
    miarex/mgt/PlaneTableDelegate.h \
    miarex/mgt/XmlPlaneReader.h \
    miarex/mgt/XmlPlaneWriter.h \
    miarex/mgt/xmlwpolarreader.h \
    miarex/mgt/xmlwpolarwriter.h \
    miarex/objects3d.h \
    miarex/view/GL3DScales.h \
    miarex/view/GLLightDlg.h \
    miarex/view/StabViewDlg.h \
    miarex/view/TargetCurveDlg.h \
    miarex/view/W3dPrefsDlg.h \
    misc/AboutQ5.h \
    misc/EditPlrDlg.h \
    misc/LengthUnitDlg.h \
    misc/ModDlg.h \
    misc/NewNameDlg.h \
    misc/ObjectPropsDlg.h \
    misc/PolarFilterDlg.h \
    misc/ProgressDlg.h \
    misc/RenameDlg.h \
    misc/color/ColorButton.h \
    misc/color/ColorButton.h \
    misc/exponentialslider.h \
    misc/line/LineBtn.h \
    misc/line/LineCbBox.h \
    misc/line/LineDelegate.h \
    misc/line/LinePickerDlg.h \
    misc/options/displayoptions.h \
    misc/options/languagewt.h \
    misc/options/preferencesdlg.h \
    misc/options/saveoptions.h \
    misc/options/units.h \
    misc/popup.h \
    misc/stlexportdialog.h \
    misc/text/DoubleEdit.h \
    misc/text/DoubleEdit.h \
    misc/text/FloatEditDelegate.h \
    misc/text/IntEdit.h \
    misc/text/MinTextEdit.h \
    misc/text/TextClrBtn.h \
    misc/updater.h \
    misc/voidwidget.h \
    misc/waitdlg.h \
    script/xflscriptexec.h \
    script/xflscriptreader.h \
    viewwidgets/glWidgets/ArcBall.h \
    viewwidgets/glWidgets/gl3dbodyview.h \
    viewwidgets/glWidgets/gl3dmiarexview.h \
    viewwidgets/glWidgets/gl3dplaneview.h \
    viewwidgets/glWidgets/gl3dtestview.h \
    viewwidgets/glWidgets/gl3dview.h \
    viewwidgets/glWidgets/gl3dwingview.h \
    viewwidgets/graphtilewidget.h \
    viewwidgets/graphwidget.h \
    viewwidgets/inverseviewwidget.h \
    viewwidgets/legendwidget.h \
    viewwidgets/miarextilewidget.h \
    viewwidgets/oppointwidget.h \
    viewwidgets/section2dwidget.h \
    viewwidgets/wingwidget.h \
    viewwidgets/xdirecttilewidget.h \
    xdirect/ManageFoilsDlg.h \
    xdirect/XDirect.h \
    xdirect/XDirectStyleDlg.h \
    xdirect/analysis/BatchDlg.h \
    xdirect/analysis/BatchThreadDlg.h \
    xdirect/analysis/FoilPolarDlg.h \
    xdirect/analysis/ReListDlg.h \
    xdirect/analysis/XFoilAdvancedDlg.h \
    xdirect/analysis/XFoilAnalysisDlg.h \
    xdirect/analysis/XFoilTask.h \
    xdirect/analysis/xfoiltaskevent.h \
    xdirect/geometry/CAddDlg.h \
    xdirect/geometry/FlapDlg.h \
    xdirect/geometry/FoilCoordDlg.h \
    xdirect/geometry/FoilGeomDlg.h \
    xdirect/geometry/InterpolateFoilsDlg.h \
    xdirect/geometry/LEDlg.h \
    xdirect/geometry/NacaFoilDlg.h \
    xdirect/geometry/TEGapDlg.h \
    xdirect/geometry/TwoDPanelDlg.h \
    xdirect/objects2d.h \
    xdirect/xmlpolarreader.h \
    xdirect/xmlpolarwriter.h \
    xinverse/FoilSelectionDlg.h \
    xinverse/InverseOptionsDlg.h \
    xinverse/PertDlg.h \
    xinverse/XInverse.h \
    $$PWD/viewwidgets/foildesignwt.h \
    $$PWD/viewwidgets/bodyframewt.h \
    $$PWD/viewwidgets/bodylinewt.h


RESOURCES += \
    scripts.qrc \
    images.qrc \
    shaders.qrc \
    textures.qrc


