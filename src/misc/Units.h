/****************************************************************************

	UnitsDlg Class
	Copyright (C) 2009 Andre Deperrois adeperrois@xflr5.com

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


#ifndef UNITSDLG_H
#define UNITSDLG_H

#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QLabel>

class Units : public QDialog
{
	Q_OBJECT
	friend class MainFrame;
	friend class QMiarex;

public:
    Units(QWidget *parent);

	static void getLengthUnitLabel(QString &str);
	static void getSpeedUnitLabel( QString &str);
	static void getWeightUnitLabel(QString &str);
	static void getAreaUnitLabel(  QString &str);
	static void getMomentUnitLabel(QString &str);
	static void getForceUnitLabel( QString &str);
	static void getPressureUnitLabel( QString &str);

	static QString lengthUnitLabel();
    static QString speedUnitLabel();
    static QString weightUnitLabel();
    static QString areaUnitLabel();
    static QString momentUnitLabel();
	static QString forceUnitLabel();
	static QString pressureUnitLabel();

    static double mtoUnit()  {return s_mtoUnit;}
    static double mstoUnit() {return s_mstoUnit;}
    static double m2toUnit() {return s_m2toUnit;}
    static double kgtoUnit() {return s_kgtoUnit;}
    static double NtoUnit()  {return s_NtoUnit;}
	static double NmtoUnit() {return s_NmtoUnit;}
	static double PatoUnit() {return s_PatoUnit;}

	static int lengthUnitIndex()   {return s_LengthUnit;}
	static int areaUnitIndex()     {return s_AreaUnit;}
	static int weightUnitIndex()   {return s_WeightUnit;}
	static int speedUnitIndex()    {return s_SpeedUnit;}
	static int forceUnitIndex()    {return s_ForceUnit;}
	static int momentUnitIndex()   {return s_MomentUnit;}
	static int pressureUnitIndex() {return s_PressureUnit;}

	static void setLengthUnitFactor(   double mToUnit)  {s_mtoUnit      = mToUnit;}
	static void setAreaUnitFactor(     double m2ToUnit) {s_m2toUnit     = m2ToUnit;}
	static void setWeightUnitFactor(   double kgToUnit) {s_kgtoUnit     = kgToUnit;}
	static void setSpeedhUnitFactor(   double msToUnit) {s_SpeedUnit    = msToUnit;}
	static void setForcehUnitFactor(   double NToUnit)  {s_ForceUnit    = NToUnit;}
	static void setMomenthUnitFactor(  double NmToUnit) {s_MomentUnit   = NmToUnit;}
	static void setPressurehUnitFactor(double PaToUnit) {s_PressureUnit = PaToUnit;}

    static void setUnitConversionFactors();

private slots:
	void onSelChanged(const QString &);

private:
	QPushButton *OKButton, *CancelButton;
	QComboBox	*m_pctrlMoment;
	QComboBox	*m_pctrlSurface;
	QComboBox	*m_pctrlWeight;
	QComboBox	*m_pctrlSpeed;
	QComboBox	*m_pctrlLength;
	QComboBox	*m_pctrlForce;
	QComboBox   *m_pctrlPressure;
	QLabel *m_pctrlForceFactor,    *m_pctrlForceInvFactor;
	QLabel *m_pctrlLengthFactor,   *m_pctrlLengthInvFactor;
	QLabel *m_pctrlSpeedFactor,    *m_pctrlSpeedInvFactor;
	QLabel *m_pctrlSurfaceFactor,  *m_pctrlSurfaceInvFactor;
	QLabel *m_pctrlWeightFactor,   *m_pctrlWeightInvFactor;
	QLabel *m_pctrlMomentFactor,   *m_pctrlMomentInvFactor;
	QLabel *m_pctrlPressureFactor, *m_pctrlPressureInvFactor;
	QLabel *m_pctrlQuestion;

private:
	void initDialog();
	void setupLayout();

	bool m_bLengthOnly;
	static int s_LengthUnit;    /**< The index of the custom unit in the array of length units. @todo use an enumeration instead. */
	static int s_AreaUnit;      /**< The index of the custom unit in the array of area units. */
	static int s_WeightUnit;    /**< The index of the custom unit in the array of mass units. */
	static int s_MomentUnit;    /**< The index of the custom unit in the array of moment units. */
	static int s_SpeedUnit;     /**< The index of the custom unit in the array of speed units. */
	static int s_ForceUnit;     /**< The index of the custom unit in the array of force units. */
	static int s_PressureUnit;  /**< The index of the custom unit in the array of pressure units. */

	static double s_mtoUnit;    /**< Conversion factor from meters to the custom length unit. */
	static double s_mstoUnit;   /**< Conversion factor from m/s to the custom speed unit. */
	static double s_m2toUnit;   /**< Conversion factor from square meters to the custom area unit. */
	static double s_kgtoUnit;   /**< Conversion factor from kg to the custom mass unit. */
	static double s_NtoUnit;    /**< Conversion factor from Newtons to the custom force unit. */
	static double s_NmtoUnit;   /**< Conversion factor from N.m to the custom unit for moments. */
	static double s_PatoUnit;   /**< Conversion factor from Pascal to the custom unit for pressures. */

public:

	QString m_Question;
};

#endif // UNITSDLG_H
