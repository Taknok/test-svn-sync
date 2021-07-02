/****************************************************************************

    Settings Class
    Copyright (C) 2018 André Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/



#pragma once

#include <QDir>
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSettings>

#include <xflcore/core_enums.h>
#include <xflgraph/graph.h>

class MainFrame;
class TextClrBtn;
class ColorBtn;

// first name space
namespace SETTINGS
{
/** @enum The different types of polar available for 2D and 3D calculations. */
    typedef enum {LIGHTTHEME, DARKTHEME, CUSTOMTHEME} enumThemeType;
}


class Settings : public QWidget
{
    Q_OBJECT

    friend class MainFrame;

    public:
        Settings(QWidget *pParent);
        void initWidget();

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

        static QString &styleName(){return s_StyleName;}
        static void setColorList();
        static bool isLightTheme() {return s_Theme==SETTINGS::LIGHTTHEME;}

        static void setLastDirName(QString dirname) {s_LastDirName=dirname;}
        static QString const &lastDirName() {return s_LastDirName;}

        static void setXmlDirName(QString dirname) {s_xmlDirName=dirname;}
        static QString xmlDirName() {return s_xmlDirName;}

        static void setPlrDirName(QString dirname) {s_plrDirName=dirname;}
        static QString plrDirName() {return s_plrDirName;}

        static void setAlignedChildrenStyle(bool bAlign) {s_bAlignChildrenStyle = bAlign;}
        static bool isAlignedChildrenStyle() {return s_bAlignChildrenStyle;}

        static void showMousePos(bool bShow) {s_bShowMousePos=bShow;}
        static bool bMousePos() {return s_bShowMousePos;}

    private slots:
        void onAlignChildrenStyle();
        void onBackgroundColor2d();
        void onGraphSettings();
        void onReverseZoom();
        void onStyleChanged(const QString &StyleName);
        void onTableFont();
        void onTextColor();
        void onTextFont();
        void onTheme();

    private:
        void setupLayout();
        void setButtonFonts();

    private:
        ColorBtn *m_pcbBackColor;
        TextClrBtn *m_ptcbTextClr;
        QPushButton *m_ppbTextFont, *m_ppbTableFont;
        QPushButton *m_ppbGraphSettings;

        QCheckBox *m_pchReverseZoom;
        QCheckBox *m_pchAlignChildrenStyle;

        QComboBox *m_pcbStyles;

        QRadioButton *m_prbDark, *m_prbLight, *m_prbCustom;

        QDir m_StyleSheetDir;
        Graph m_MemGraph;
        bool m_bIsGraphModified;

    public:
        //settings variables used throughout the program
        static QString s_StyleName, s_StyleSheetName;

        static bool s_bStyleSheets;

        static xfl::enumTextFileType s_ExportFileType;  /**< Defines if the list separator for the output text files should be a space or a comma. */
        static Graph s_RefGraph;//Reference setttings
        static QString s_LastDirName, s_xmlDirName, s_plrDirName;
        static QStringList s_colorList;
        static QStringList s_colorNames;
        static SETTINGS::enumThemeType s_Theme;
        static bool s_bShowMousePos;

        static bool s_bAlignChildrenStyle;
};


