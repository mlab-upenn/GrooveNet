/***************************************************************************
 *   Copyright (C) 2005, Carnegie Mellon University.                       *
 *   Maintained by: Daniel Weller                                          *
 *                  Rahul Mangharam                                        *
 *                  and the rest of the GrooveNet Team                     *
 *                                                                         *
 *   Email: dweller@ece.cmu.edu or rahulm@ece.cmu.edu                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _SIMPLELINKMODEL_H
#define _SIMPLELINKMODEL_H

#include "CarModel.h"

#define SIMPLELINKMODEL_NAME "SimpleLinkModel"

class SimpleLinkModel : public CarLinkModel
{
public:
	inline virtual QString GetModelType() const
	{
		return SIMPLELINKMODEL_NAME;
	}
	inline virtual bool IsModelTypeOf(const QString & strModelType, bool bDescendSufficient = true) const
	{
		return strModelType.compare(SIMPLELINKMODEL_NAME) == 0 || (bDescendSufficient && CarLinkModel::IsModelTypeOf(strModelType, bDescendSufficient));
	}

	SimpleLinkModel(const QString & strModelName = QString::null);
	SimpleLinkModel(const SimpleLinkModel & copy);
	virtual ~SimpleLinkModel();

	virtual SimpleLinkModel & operator = (const SimpleLinkModel & copy);

	static void GetParams(std::map<QString, ModelParameter> & mapParams);
	
	virtual bool ReceivePacket(Packet * packet);
	virtual bool BeginProcessPacket(Packet * packet);
	virtual bool EndProcessPacket(Packet * packet);

protected:
};

inline Model * SimpleLinkModelCreator(const QString & strModelName)
{
	return new SimpleLinkModel(strModelName);
}

#endif
