/***************************************************************************
    File                 : Origin750Parser.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 Alex Kargovsky, Stefan Gerlach, 
						   Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin 7.5 file parser class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "Origin750Parser.h"
#include <cstring>
#include <sstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <logging.hpp>

using namespace boost;

const char* colTypeNames[] = {"X", "Y", "Z", "XErr", "YErr", "Label", "None"};

inline boost::posix_time::ptime doubleToPosixTime(double jdt)
{
	return boost::posix_time::ptime(boost::gregorian::date(boost::gregorian::gregorian_calendar::from_julian_day_number(jdt+1)), boost::posix_time::seconds((jdt-(int)jdt)*86400));
}

Origin750Parser::Origin750Parser(const string& fileName)
:	file(fileName.c_str(), ios::binary)
{
	objectIndex = 0;
}

bool Origin750Parser::parse()
{
	unsigned int dataIndex = 0;

	stringstream out;
	unsigned char c;
	/////////////////// find column ///////////////////////////////////////////////////////////
	file.seekg(0x10 + 1, ios_base::beg);
	unsigned int size;
	file >> size;
	file.seekg(1 + size + 1 + 5, ios_base::cur);

	file >> size;

	file.seekg(1, ios_base::cur);
	BOOST_LOG_(1, format("	[column found = %d/0x%X @ 0x%X]") % size % size % (unsigned int) file.tellg());

	unsigned int colpos = file.tellg();
	unsigned int current_col = 1, nr = 0, nbytes = 0;
	
	while(size > 0 && size < 0x84) {	// should be 0x72, 0x73 or 0x83
		//////////////////////////////// COLUMN HEADER /////////////////////////////////////////////
		short data_type;
		char data_type_u;
		unsigned int oldpos = file.tellg();

		file.seekg(oldpos + 0x16, ios_base::beg);
		file >> data_type;

		file.seekg(oldpos + 0x3F, ios_base::beg);
		file >> data_type_u;
		
		char valuesize;
		file.seekg(oldpos + 0x3D, ios_base::beg);
		file >> valuesize;

		BOOST_LOG_(1, format("	[valuesize = %d @ 0x%X]") % (int)valuesize % ((unsigned int) file.tellg()-1));
		if(valuesize <= 0)
		{
			BOOST_LOG_(1, format("	WARNING : found strange valuesize of %d") % (int)valuesize);
			valuesize = 10;
		}

		file.seekg(oldpos + 0x58, ios_base::beg);
		BOOST_LOG_(1, format("	[Spreadsheet @ 0x%X]") % (unsigned int) file.tellg());

		string name(25, 0);
		file >> name;

		string::size_type pos = name.find_last_of("_");
		string columnname;
		if(pos != string::npos)
		{
			columnname = name.substr(pos + 1);
			name.resize(pos);
		}

		unsigned int spread = 0;
		if(columnname.empty())
		{
			BOOST_LOG_(1, "NO COLUMN NAME FOUND! Must be a Matrix or Function.");
			////////////////////////////// READ matrixes or functions ////////////////////////////////////

			BOOST_LOG_(1, format("	[position @ 0x%X]") % (unsigned int) file.tellg());
			// TODO
			short signature;
			file >> signature;
			BOOST_LOG_(1, format("	SIGNATURE : %02X ") % signature);


			file.seekg(oldpos + size + 1, ios_base::beg);
			file >> size;
			file.seekg(1, ios_base::cur);
			size /= valuesize;
			BOOST_LOG_(1, format("	SIZE = %d") % size);

			// catch exception
			/*if(size>10000)
			size=1000;*/

			switch(signature)
			{
			case 0x50CA:
			case 0x70CA:
			case 0x50F2:
			case 0x50E2:
				BOOST_LOG_(1, "NEW MATRIX");
				matrixes.push_back(Matrix(name, dataIndex));
				++dataIndex;

				BOOST_LOG_(1, "VALUES :");
				out.str(string());
				switch(data_type)
				{
				case 0x6001://double
					for(unsigned int i = 0; i < size; ++i)
					{
						double value;
						//fread(&value,valuesize,1,f);
						
						file >> value;
						matrixes.back().data.push_back((double)value);
						out << format("%g ") % matrixes.back().data.back();
					}
					BOOST_LOG_(1, out.str());
					break;
				case 0x6003://float
					for(unsigned int i = 0; i < size; ++i)
					{
						float value;
						//fread(&value,valuesize,1,f);
						
						file >> value;
						matrixes.back().data.push_back((double)value);
						out << format("%g ") % matrixes.back().data.back();
					}
					BOOST_LOG_(1, out.str());
					break;
				case 0x6801://int
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned int value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							int value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				case 0x6803://short
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned short value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							short value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				case 0x6821://char
					if(data_type_u == 8)//unsigned
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							unsigned char value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					else
					{
						for(unsigned int i = 0; i < size; ++i)
						{
							char value;
							//fread(&value,valuesize,1,f);
							
							file >> value;
							matrixes.back().data.push_back((double)value);
							out << format("%g ") % matrixes.back().data.back();
						}
						BOOST_LOG_(1, out.str());
					}
					break;
				default:
					BOOST_LOG_(1, format("UNKNOWN MATRIX DATATYPE: %02X SKIP DATA") % data_type);
					file.seekg(valuesize*size, ios_base::cur);
					matrixes.pop_back();
				}
				break;
			case 0x10C8:
				BOOST_LOG_(1, "NEW FUNCTION");
				functions.push_back(Function(name, dataIndex));
				++dataIndex;

				file >> functions.back().formula.assign(valuesize, 0);
				oldpos = file.tellg();
				short t;

				file.seekg(colpos + 0xA, ios_base::beg);
				file >> t;

				if(t == 0x1194)
					functions.back().type = Function::Polar;

				file.seekg(colpos + 0x21, ios_base::beg);
				file >> functions.back().totalPoints;

				file >> functions.back().begin;
				double d;
				file >> d;
				functions.back().end = functions.back().begin + d*(functions.back().totalPoints - 1);

				BOOST_LOG_(1, format("FUNCTION %s : %s") % functions.back().name.c_str() % functions.back().formula.c_str());
				BOOST_LOG_(1, format(" interval %g : %g, number of points %d") % functions.back().begin % functions.back().end % functions.back().totalPoints);

				file.seekg(oldpos, ios_base::beg);
				break;
			default:
				BOOST_LOG_(1, format("UNKNOWN SIGNATURE: %.2X SKIP DATA") % signature);
				file.seekg(valuesize*size, ios_base::cur);

				if(valuesize != 8 && valuesize <= 16)
				{
					file.seekg(2, ios_base::cur);
				}
			}
		}
		else
		{	// worksheet
			if(speadSheets.size() == 0 || findSpreadByName(name) == -1)
			{
				BOOST_LOG_(1, "NEW SPREADSHEET");
				current_col = 1;
				speadSheets.push_back(SpreadSheet(name));
				spread = speadSheets.size() - 1;
				speadSheets.back().maxRows = 0;
			}
			else
			{
				spread = findSpreadByName(/*sname*/name);

				current_col = speadSheets[spread].columns.size();

				if(!current_col)
					current_col = 1;
				++current_col;
			}
			BOOST_LOG_(1, format("SPREADSHEET = %s COLUMN NAME = %s (%d) (@0x%X)") % name % columnname % current_col % (unsigned int)file.tellg());
			speadSheets[spread].columns.push_back(SpreadColumn(columnname, dataIndex));
			string::size_type sheetpos = speadSheets[spread].columns.back().name.find_last_of("@");
			if(!speadSheets[spread].multisheet && sheetpos != string::npos)
			{
				if(lexical_cast<int>(columnname.substr(sheetpos + 1).c_str()) > 1)
				{
					speadSheets[spread].multisheet = true;
					BOOST_LOG_(1, format("SPREADSHEET \"%s\" IS MULTISHEET") % name);
				}
			}
			++dataIndex;

			////////////////////////////// SIZE of column /////////////////////////////////////////////
			file.seekg(oldpos + size + 1, ios_base::beg);

			file >> nbytes;
			if(fmod(nbytes, (double)valuesize)>0)
			{
				BOOST_LOG_(1, "WARNING: data section could not be read correct");
			}
			nr = nbytes / valuesize;
			BOOST_LOG_(1, format("	[number of rows = %d (%d Bytes) @ 0x%X]") % nr % nbytes % (unsigned int)file.tellg());

			speadSheets[spread].maxRows<nr ? speadSheets[spread].maxRows=nr : 0;

			////////////////////////////////////// DATA ////////////////////////////////////////////////
			file.seekg(1, ios_base::cur);

			BOOST_LOG_(1, format("	[data @ 0x%X]") % (unsigned int)file.tellg());
			out.str(string());
			for(unsigned int i = 0; i < nr; ++i)
			{
				double value;
				if(valuesize <= 8)	// Numeric, Time, Date, Month, Day
				{
					file >> value;
					out << format("%g ") % value;
					speadSheets[spread].columns[(current_col-1)].data.push_back(value);
				}
				else if((data_type & 0x100) == 0x100) // Text&Numeric
				{
					file >> c;
					file.seekg(1, ios_base::cur);
					if(c == 0) //value
					{
						file >> value;
						out << format("%g ") % value;
						speadSheets[spread].columns[(current_col-1)].data.push_back(value);
						file.seekg(valuesize - 10, ios_base::cur);
					}
					else //text
					{
						string stmp(valuesize - 2, 0);
						file >> stmp;
						if(stmp.find(0x0E) != string::npos) // try find non-printable symbol - garbage test
							stmp = string();
						speadSheets[spread].columns[(current_col-1)].data.push_back(stmp);
						out << format("%s ") % stmp;
					}
				}
				else //text
				{
					string stmp(valuesize, 0);
					file >> stmp;
					if(stmp.find(0x0E) != string::npos) // try find non-printable symbol - garbage test
						stmp = string();
					speadSheets[spread].columns[(current_col-1)].data.push_back(stmp);
					out << format("%s ") % stmp;
				}
			}
			BOOST_LOG_(1, out.str());
		}

		if(nbytes > 0 || columnname.empty())
		{
			file.seekg(1, ios_base::cur);
		}

		file >> size;
		file.seekg(1 + size + (size > 0 ? 1 : 0), ios_base::cur);

		file >> size;

		file.seekg(1, ios_base::cur);
		BOOST_LOG_(1, format("	[column found = %d/0x%X (@ 0x%X)]") % size % size %((unsigned int) file.tellg()-5));
		colpos = file.tellg();
	}

	////////////////////////////////////////////////////////////////////////////
	for(unsigned int i = 0; i < speadSheets.size(); ++i)
	{
		if(speadSheets[i].multisheet)
		{
			BOOST_LOG_(1, format("		CONVERT SPREADSHEET \"%s\" to EXCEL") % speadSheets[i].name.c_str());
			convertSpreadToExcel(i);
			--i;
		}
	}
	////////////////////////////////////////////////////////////////////////////
	////////////////////// HEADER SECTION //////////////////////////////////////

	unsigned int POS = (unsigned int)file.tellg()-11;
	BOOST_LOG_(1, "\nHEADER SECTION");
	BOOST_LOG_(1, format("	nr_spreads = %d") % speadSheets.size());
	BOOST_LOG_(1, format("	[position @ 0x%X]") % POS);

	//////////////////////// OBJECT INFOS //////////////////////////////////////
	POS += 0xB;
	file.seekg(POS, ios_base::beg);
	while(1)
	{
		BOOST_LOG_(1, "			reading	Header");
		// HEADER
		// check header
		POS = file.tellg();

		file >> size;
		if(size == 0)
			break;

		file.seekg(POS + 0x7, ios_base::beg);
		string name(25, 0);
		file >> name;

		file.seekg(POS, ios_base::beg);

		if(findSpreadByName(name) != -1)
			readSpreadInfo();
		else if(findMatrixByName(name) != -1)
			readMatrixInfo();
		else if(findExcelByName(name) != -1)
			readExcelInfo();
		else
			readGraphInfo();
	}

	file.seekg(1, ios_base::cur);
	BOOST_LOG_(1, format("Some Origin params @ 0x%X:") % (unsigned int)file.tellg());

	file >> c;
	while(c != 0)
	{
		out.str(string());
		out << "		";
		while(c != '\n')
		{
			out << c;
			file >> c;
		}
		double parvalue;
		file >> parvalue;
		out << format(": %g") % parvalue;
		BOOST_LOG_(1, out.str());

		file.seekg(1, ios_base::cur);
		file >> c;
	}
	file.seekg(1 + 5, ios_base::cur);
	while(1)
	{
		//fseek(f,5+0x40+1,SEEK_CUR);
		int size;
		file >> size;
		if(size != 0x40)
			break;

		file.seekg(1, ios_base::cur);
		Rect rect;
		unsigned int coord;
		file >> coord;
		rect.left = coord;
		file >> coord;
		rect.top = coord;
		file >> coord;
		rect.right = coord;
		file >> coord;
		rect.bottom = coord;

		unsigned char state;
		file.seekg(0x8, ios_base::cur);
		file >> state;

		double creationDate, modificationDate;
		file.seekg(0x7, ios_base::cur);
		file >> creationDate;
		file >> modificationDate;

		file.seekg(0x8, ios_base::cur);
		file >> c;

		unsigned char labellen;
		file.seekg(0x3, ios_base::cur);
		file >> labellen;

		file.seekg(4, ios_base::cur);
		file >> size;

		file.seekg(1, ios_base::cur);

		string name(size, 0);
		file >> name;

		if(name == "ResultsLog")
		{
			file.seekg(1, ios_base::cur);
			file >> size;

			file.seekg(1, ios_base::cur);
			resultsLog.resize(size);
			file >> resultsLog;

			BOOST_LOG_(1, format("Results Log: %s") % resultsLog);
			break;
		}
		else
		{
			notes.push_back(Note(name));
			notes.back().objectID = objectIndex;
			notes.back().frameRect = rect;
			notes.back().creationDate = doubleToPosixTime(creationDate);
			notes.back().modificationDate = doubleToPosixTime(modificationDate);
			
			if(c & 0x01)
				notes.back().title = Window::Label;
			else if(c & 0x02)
				notes.back().title = Window::Name;
			else
				notes.back().title = Window::Both;

			notes.back().hidden = (state & 0x40);

			++objectIndex;

			file.seekg(1, ios_base::cur);
			file >> size;

			file.seekg(1, ios_base::cur);

			if(labellen > 1)
			{
				file >> notes.back().label.assign(labellen-1, 0);
				file.seekg(1, ios_base::cur);
			}

			file >> notes.back().text.assign(size - labellen, 0);

			BOOST_LOG_(1, format("NOTE %d NAME: %s") % notes.size() % notes.back().name);
			BOOST_LOG_(1, format("NOTE %d LABEL: %s") % notes.size() % notes.back().label);
			BOOST_LOG_(1, format("NOTE %d TEXT: %s") % notes.size() % notes.back().text);

			file.seekg(1, ios_base::cur);
		}
	}

	file.seekg(1 + 4*5 + 0x10 + 1, ios_base::cur);
	try
	{
		readProjectTree();
	}
	catch(...)
	{}
	BOOST_LOG_(1, "Done parsing");
	BOOST_LOG_FINALIZE();

	return true;
}

void Origin750Parser::readSpreadInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS += 5;

	BOOST_LOG_(1, format("			[Spreadsheet SECTION (@ 0x%X)]") % POS);

	// check spreadsheet name
	file.seekg(POS + 0x2, ios_base::beg);
	string name(25, 0);
	file >> name;

	int spread = findSpreadByName(name);
	speadSheets[spread].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(speadSheets[spread], size);
	speadSheets[spread].loose = false;
	char c = 0;

	unsigned int LAYER = POS;
	{
		// LAYER section
		LAYER += size + 0x1 + 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header
			file.seekg(LAYER + 0x46, ios_base::beg);
			string sec_name(41, 0);
			file >> sec_name;

			BOOST_LOG_(1, format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46));

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			file.seekg(LAYER, ios_base::beg);
			//check if it is a formula
			int col_index = findSpreadColumnByName(spread, sec_name);
			if(col_index != -1)
			{
				file >> speadSheets[spread].columns[col_index].command.assign(size, 0);
			}

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;
	}

	/////////////// COLUMN Types ///////////////////////////////////////////
	BOOST_LOG_(1, format("			Spreadsheet has %d columns") % speadSheets[spread].columns.size());

	while(1)
	{
		LAYER += 0x5;
		file.seekg(LAYER + 0x12, ios_base::beg);
		name.resize(12);
		file >> name;

		file.seekg(LAYER + 0x11, ios_base::beg);
		file >> c;

		short width=0;
		file.seekg(LAYER + 0x4A, ios_base::beg);
		file >> width;
		int col_index = findSpreadColumnByName(spread, name);
		if(col_index != -1)
		{
			SpreadColumn::ColumnType type;
			switch(c)
			{
				case 3:
					type = SpreadColumn::X;
					break;
				case 0:
					type = SpreadColumn::Y;
					break;
				case 5:
					type = SpreadColumn::Z;
					break;
				case 6:
					type = SpreadColumn::XErr;
					break;
				case 2:
					type = SpreadColumn::YErr;
					break;
				case 4:
					type = SpreadColumn::Label;
					break;
				default:
					type = SpreadColumn::NONE;
					break;
			}
			speadSheets[spread].columns[col_index].type = type;
			width/=0xA;
			if(width == 0)
				width = 8;
			speadSheets[spread].columns[col_index].width = width;

			unsigned char c1,c2;
			file.seekg(LAYER + 0x1E, ios_base::beg);
			file >> c1;
			file >> c2;

			switch(c1)
			{
			case 0x00: // Numeric	   - Dec1000
			case 0x09: // Text&Numeric - Dec1000
			case 0x10: // Numeric	   - Scientific
			case 0x19: // Text&Numeric - Scientific
			case 0x20: // Numeric	   - Engeneering
			case 0x29: // Text&Numeric - Engeneering
			case 0x30: // Numeric	   - Dec1,000
			case 0x39: // Text&Numeric - Dec1,000
				speadSheets[spread].columns[col_index].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c1 / 0x10;
				if(c2 >= 0x80)
				{
					speadSheets[spread].columns[col_index].significantDigits = c2 - 0x80;
					speadSheets[spread].columns[col_index].numericDisplayType = SignificantDigits;
				}
				else if(c2 > 0)
				{
					speadSheets[spread].columns[col_index].decimalPlaces = c2 - 0x03;
					speadSheets[spread].columns[col_index].numericDisplayType = DecimalPlaces;
				}
				break;
			case 0x02: // Time
				speadSheets[spread].columns[col_index].valueType = Time;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2 - 0x80;
				break;
			case 0x03: // Date
				speadSheets[spread].columns[col_index].valueType = Date;
				speadSheets[spread].columns[col_index].valueTypeSpecification= c2 - 0x80;
				break;
			case 0x31: // Text
				speadSheets[spread].columns[col_index].valueType = Text;
				break;
			case 0x4: // Month
			case 0x34:
				speadSheets[spread].columns[col_index].valueType = Month;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2;
				break;
			case 0x5: // Day
			case 0x35:
				speadSheets[spread].columns[col_index].valueType = Day;
				speadSheets[spread].columns[col_index].valueTypeSpecification = c2;
				break;
			default: // Text
				speadSheets[spread].columns[col_index].valueType = Text;
				break;
			}
			BOOST_LOG_(1, format("				COLUMN \"%s\" type = %s(%d) (@ 0x%X)") % speadSheets[spread].columns[col_index].name.c_str() % colTypeNames[type] % (int)c % (LAYER + 0x11));
		}
		LAYER += 0x1E7 + 0x1;

		int size;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		LAYER += 0x5;
		if(size > 0)
		{
			if(col_index != -1)
			{
				file.seekg(LAYER, ios_base::beg);
				file >> speadSheets[spread].columns[col_index].comment.assign(size, 0);
			}
			LAYER += size + 0x1;
		}

		file.seekg(LAYER, ios_base::beg);
		file >> size;
		if(size != 0x1E7)
			break;
	}
	BOOST_LOG_(1, format("		Done with spreadsheet %d") % spread);

	file.seekg(LAYER + 0x5*0x6 + 0x1ED*0x12, ios_base::beg);
}

void Origin750Parser::readExcelInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS += 5;

	BOOST_LOG_(1, format("			[EXCEL SECTION (@ 0x%X)]") % POS);

	// check spreadsheet name
	string name(25, 0);
	file.seekg(POS + 0x2, ios_base::beg);
	file >> name;

	int iexcel = findExcelByName(name);
	excels[iexcel].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(excels[iexcel], size);
	excels[iexcel].loose = false;
	char c = 0;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;
	int isheet = 0;
	while(1)// multisheet loop
	{
		// LAYER section
		LAYER += 0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage etc
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			string sec_name(41, 0);
			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> sec_name;

			BOOST_LOG_(1, format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46));

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			file.seekg(LAYER, ios_base::beg);
			//check if it is a formula
			int col_index = findExcelColumnByName(iexcel, isheet, sec_name);
			if(col_index!=-1)
			{
				file >> excels[iexcel].sheets[isheet].columns[col_index].command.assign(size, 0);
			}

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;

		/////////////// COLUMN Types ///////////////////////////////////////////
		BOOST_LOG_(1, format("			Excel sheet %d has %d columns") % isheet % excels[iexcel].sheets[isheet].columns.size());

		while(1)
		{
			LAYER += 0x5;
			file.seekg(LAYER + 0x12, ios_base::beg);
			name.resize(12);
			file >> name;

			file.seekg(LAYER + 0x11, ios_base::beg);
			file >> c;

			short width=0;
			file.seekg(LAYER + 0x4A, ios_base::beg);
			file >> width;

			int col_index = findExcelColumnByName(iexcel, isheet, name);
			if(col_index != -1)
			{
				SpreadColumn::ColumnType type;
				switch(c)
				{
					case 3:
						type = SpreadColumn::X;
						break;
					case 0:
						type = SpreadColumn::Y;
						break;
					case 5:
						type = SpreadColumn::Z;
						break;
					case 6:
						type = SpreadColumn::XErr;
						break;
					case 2:
						type = SpreadColumn::YErr;
						break;
					case 4:
						type = SpreadColumn::Label;
						break;
					default:
						type = SpreadColumn::NONE;
						break;
				}
				excels[iexcel].sheets[isheet].columns[col_index].type = type;
				width/=0xA;
				if(width == 0)
					width = 8;
				excels[iexcel].sheets[isheet].columns[col_index].width = width;

				unsigned char c1,c2;
				file.seekg(LAYER + 0x1E, ios_base::beg);
				file >> c1;
				file >> c2;
				switch(c1)
				{
				case 0x00: // Numeric	   - Dec1000
				case 0x09: // Text&Numeric - Dec1000
				case 0x10: // Numeric	   - Scientific
				case 0x19: // Text&Numeric - Scientific
				case 0x20: // Numeric	   - Engeneering
				case 0x29: // Text&Numeric - Engeneering
				case 0x30: // Numeric	   - Dec1,000
				case 0x39: // Text&Numeric - Dec1,000
					excels[iexcel].sheets[isheet].columns[col_index].valueType = (c1%0x10 == 0x9) ? TextNumeric : Numeric;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c1 / 0x10;
					if(c2>=0x80)
					{
						excels[iexcel].sheets[isheet].columns[col_index].significantDigits = c2 - 0x80;
						excels[iexcel].sheets[isheet].columns[col_index].numericDisplayType = SignificantDigits;
					}
					else if(c2>0)
					{
						excels[iexcel].sheets[isheet].columns[col_index].decimalPlaces = c2 - 0x03;
						excels[iexcel].sheets[isheet].columns[col_index].numericDisplayType = DecimalPlaces;
					}
					break;
				case 0x02: // Time
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Time;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2 - 0x80;
					break;
				case 0x03: // Date
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Date;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2 - 0x80;
					break;
				case 0x31: // Text
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Text;
					break;
				case 0x4: // Month
				case 0x34:
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Month;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2;
					break;
				case 0x5: // Day
				case 0x35:
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Day;
					excels[iexcel].sheets[isheet].columns[col_index].valueTypeSpecification = c2;
					break;
				default: // Text
					excels[iexcel].sheets[isheet].columns[col_index].valueType = Text;
					break;
				}
				BOOST_LOG_(1, format("				COLUMN \"%s\" type = %s(%d) (@ 0x%X)") % excels[iexcel].sheets[isheet].columns[col_index].name.c_str() % type % (int)c % (LAYER + 0x11));
			}
			LAYER += 0x1E7 + 0x1;

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			LAYER += 0x5;
			if(size > 0)
			{
				if(col_index != -1)
				{
					file.seekg(LAYER, ios_base::beg);
					file >> excels[iexcel].sheets[isheet].columns[col_index].comment.assign(size, 0);
				}
				LAYER += size + 0x1;
			}

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			if(size != 0x1E7)
				break;
		}
		BOOST_LOG_(1, format("		Done with Excel %d") % iexcel);

		//POS = LAYER+0x5*0x6+0x1ED*0x12;
		LAYER += 0x5*0x5 + 0x1ED*0x12;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;

		++isheet;
	}

	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin750Parser::readMatrixInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;

	POS+=5;

	BOOST_LOG_(1, format("			[Matrix SECTION (@ 0x%X)]") % POS);

	string name(25, 0);
	file.seekg(POS + 0x2, ios_base::beg);
	file >> name;

	int idx = findMatrixByName(name);
	matrixes[idx].name = name;
	file.seekg(POS, ios_base::beg);
	readWindowProperties(matrixes[idx], size);

	unsigned char h;
	file.seekg(POS + 0x87, ios_base::beg);
	file >> h;

	switch(h)
	{
	case 1:
		matrixes[idx].view = Matrix::ImageView;
		break;
	case 2:
		matrixes[idx].header = Matrix::XY;
		break;
	}

	unsigned int LAYER = POS;
	LAYER += size + 0x1;

	// LAYER section
	LAYER += 0x5;
	
	file.seekg(LAYER + 0x2B, ios_base::beg);
	file >> matrixes[idx].columnCount;

	file.seekg(LAYER + 0x52, ios_base::beg);
	file >> matrixes[idx].rowCount;

	LAYER += 0x12D + 0x1;
	//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
	//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
	//section name(column name in formula case) starts with 0x46 position
	while(1)
	{
		//section_header_size=0x6F(4 bytes) + '\n'
		LAYER += 0x5;

		//section_header
		string sec_name(41, 0);
		file.seekg(LAYER + 0x46, ios_base::beg);
		file >> sec_name;

		//section_body_1_size
		LAYER += 0x6F+0x1;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		//section_body_1
		LAYER += 0x5;
		//check if it is a formula
		if(sec_name == "MV")
		{
			file.seekg(LAYER, ios_base::beg);
			file >> matrixes[idx].command.assign(size, 0);
		}

		//section_body_2_size
		LAYER += size + 0x1;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		//section_body_2
		LAYER += 0x5;

		//close section 00 00 00 00 0A
		LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

		if(sec_name == "__LayerInfoStorage")
			break;

	}
	LAYER += 0x5;

	while(1)
	{
		LAYER+=0x5;

		unsigned short width;
		file.seekg(LAYER + 0x2B, ios_base::beg);
		file >> width;

		width = (width-55)/0xA;
		if(width == 0)
			width = 8;
		matrixes[idx].width = width;

		unsigned char c1,c2;
		file.seekg(LAYER + 0x1E, ios_base::beg);
		file >> c1;
		file >> c2;

		matrixes[idx].valueTypeSpecification = c1/0x10;
		if(c2 >= 0x80)
		{
			matrixes[idx].significantDigits = c2-0x80;
			matrixes[idx].numericDisplayType = SignificantDigits;
		}
		else if(c2 > 0)
		{
			matrixes[idx].decimalPlaces = c2-0x03;
			matrixes[idx].numericDisplayType = DecimalPlaces;
		}

		LAYER += 0x1E7 + 0x1;
		
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size != 0x1E7)
			break;
	}

	file.seekg(LAYER + 0x5*0x5 + 0x1ED*0x12 + 0x5, ios_base::beg);
}

void Origin750Parser::readGraphInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;
	POS += 5;

	BOOST_LOG_(1, format("			[Graph SECTION (@ 0x%X)]") % POS);

	string name(25, 0);
	file.seekg(POS + 0x2, ios_base::beg);
	file >> name;

	graphs.push_back(Graph(name));
	file.seekg(POS, ios_base::beg);
	readWindowProperties(graphs.back(), size);

	file.seekg(POS + 0x23, ios_base::beg);
	file >> graphs.back().width;
	file >> graphs.back().height;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;

	while(1)// multilayer loop
	{
		graphs.back().layers.push_back(GraphLayer());
		// LAYER section
		LAYER += 0x5;

		file.seekg(LAYER+0xF, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.min;
		file >> graphs.back().layers.back().xAxis.max;
		file >> graphs.back().layers.back().xAxis.step;

		file.seekg(LAYER+0x2B, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.majorTicks;

		file.seekg(LAYER+0x37, ios_base::beg);
		file >> graphs.back().layers.back().xAxis.minorTicks;
		file >> graphs.back().layers.back().xAxis.scale;

		file.seekg(LAYER+0x3A, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.min;
		file >> graphs.back().layers.back().yAxis.max;
		file >> graphs.back().layers.back().yAxis.step;

		file.seekg(LAYER+0x56, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.majorTicks;

		file.seekg(LAYER+0x62, ios_base::beg);
		file >> graphs.back().layers.back().yAxis.minorTicks;
		file >> graphs.back().layers.back().yAxis.scale;

		file.seekg(LAYER+0x71, ios_base::beg);
		file.read(reinterpret_cast<char*>(&graphs.back().layers.back().clientRect), sizeof(Rect));

		LAYER += 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: axes, legend, __BC02, _202, _231, _232, __LayerInfoStorage etc
		//section name starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header

			string sec_name(41, 0);
			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> sec_name;

			BOOST_LOG_(1, format("				SECTION NAME: %s (@ 0x%X)") % sec_name % (LAYER + 0x46));

			Rect r;
			file.seekg(LAYER + 0x3, ios_base::beg);
			file.read(reinterpret_cast<char*>(&r), sizeof(Rect));

			unsigned char attach;
			file.seekg(LAYER + 0x28, ios_base::beg);
			file >> attach;

			unsigned char border;
			file >> border;

			unsigned char color;
			file.seekg(LAYER + 0x33, ios_base::beg);
			file >> color;

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;
			unsigned int osize = size;

			unsigned char type;
			file.seekg(LAYER, ios_base::beg);
			file >> type;

			//text properties
			short rotation;
			file.seekg(LAYER + 0x2, ios_base::beg);
			file >> rotation;

			unsigned char fontSize;
			file >> fontSize;

			unsigned char tab;
			file.seekg(LAYER + 0xA, ios_base::beg);
			file >> tab;

			//line properties
			unsigned char lineStyle = 0;
			double width = 0.0;
			LineVertex begin, end;
			unsigned int w = 0;

			file.seekg(LAYER + 0x12, ios_base::beg);
			file >> lineStyle;

			unsigned short w1;
			file >> w1;
			width = (double)w1/500.0;

			file.seekg(LAYER + 0x20, ios_base::beg);
			file >> begin.x;
			file >> end.x;

			file.seekg(LAYER + 0x40, ios_base::beg);
			file >> begin.y;
			file >> end.y;

			file.seekg(LAYER + 0x60, ios_base::beg);
			file >> begin.shapeType;

			file.seekg(LAYER + 0x64, ios_base::beg);
			file >> w;
			begin.shapeWidth = (double)w/500.0;

			file >> w;
			begin.shapeLength = (double)w/500.0;

			file.seekg(LAYER + 0x6C, ios_base::beg);
			file >> end.shapeType;

			file.seekg(LAYER + 0x70, ios_base::beg);
			file >> w;
			end.shapeWidth = (double)w/500.0;

			file >> w;
			end.shapeLength = (double)w/500.0;

			Figure figure;
			file.seekg(LAYER + 0x5, ios_base::beg);
			file >> w1;
			figure.width = (double)w1/500.0;

			file.seekg(LAYER + 0x8, ios_base::beg);
			file >> figure.style;

			file.seekg(LAYER + 0x42, ios_base::beg);
			file >> figure.fillAreaColor;

			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> w1;
			figure.fillAreaPatternWidth = (double)w1/500.0;

			file.seekg(LAYER + 0x4A, ios_base::beg);
			file >> figure.fillAreaPatternColor;

			file.seekg(LAYER + 0x4E, ios_base::beg);
			file >> figure.fillAreaPattern;

			unsigned char h;
			file.seekg(LAYER + 0x57, ios_base::beg);
			file >> h;
			figure.useBorderColor = (h == 0x10);


			//section_body_2_size
			LAYER += size + 0x1;

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;
			//check if it is a axis or legend

			file.seekg(1, ios_base::cur);
			if(sec_name == "XB")
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().xAxis.position = GraphAxis::Bottom;
				graphs.back().layers.back().xAxis.label = TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(sec_name == "XT")
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().xAxis.position = GraphAxis::Top;
				graphs.back().layers.back().xAxis.label = TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(sec_name == "YL")
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().yAxis.position = GraphAxis::Left;
				graphs.back().layers.back().yAxis.label = TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(sec_name == "YR")
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().yAxis.position = GraphAxis::Right;
				graphs.back().layers.back().yAxis.label = TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(sec_name == "Legend")
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().legend = TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach);
			}
			else if(sec_name == "__BCO2") // histogram
			{
				file.seekg(LAYER + 0x10, ios_base::beg);
				file >> graphs.back().layers.back().histogramBin;

				file.seekg(LAYER + 0x20, ios_base::beg);
				file >> graphs.back().layers.back().histogramEnd;
				file >> graphs.back().layers.back().histogramBegin;
			}
			else if(osize == 0x3E) // text
			{
				string text(size, 0);
				file >> text;

				graphs.back().layers.back().texts.push_back(
								TextBox(text, r, color, fontSize, rotation/10, tab, (TextBox::BorderType)(border >= 0x80 ? border-0x80 : TextBox::None), (Attach)attach));
			}
			else if(osize == 0x5E) // rectangle & circle
			{
				switch(type)
				{
				case 0:
				case 1:
					figure.type = Figure::Rectangle;
					break;
				case 2:
				case 3:
					figure.type = Figure::Circle;
					break;
				}
				figure.clientRect = r;
				figure.attach = (Attach)attach;
				figure.color = color;

				graphs.back().layers.back().figures.push_back(figure);
			}
			else if(osize == 0x78 && type == 2) // line
			{
				graphs.back().layers.back().lines.push_back(Line());
				graphs.back().layers.back().lines.back().color = color;
				graphs.back().layers.back().lines.back().clientRect = r;
				graphs.back().layers.back().lines.back().attach = (Attach)attach;
				graphs.back().layers.back().lines.back().width = width;
				graphs.back().layers.back().lines.back().style = lineStyle;
				graphs.back().layers.back().lines.back().begin = begin;
				graphs.back().layers.back().lines.back().end = end;
			}
			else if(osize == 0x28 && type == 4) // bitmap
			{
				unsigned long filesize = size + 14;
				graphs.back().layers.back().bitmaps.push_back(Bitmap());
				graphs.back().layers.back().bitmaps.back().clientRect = r;
				graphs.back().layers.back().bitmaps.back().attach = (Attach)attach;
				graphs.back().layers.back().bitmaps.back().size = filesize;
				graphs.back().layers.back().bitmaps.back().data = new unsigned char[filesize];
				unsigned char* data = graphs.back().layers.back().bitmaps.back().data;
				//add Bitmap header
				memcpy(data, "BM", 2);
				data += 2;
				memcpy(data, &filesize, 4);
				data += 4;
				unsigned int d = 0;
				memcpy(data, &d, 4);
				data += 4;
				d = 0x36;
				memcpy(data, &d, 4);
				data += 4;
				//fread(data,sec_size,1,f);
				file.read(reinterpret_cast<char*>(data), size);
			}

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			//section_body_3_size
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_3
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;
		unsigned char h;
		short w;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0x1E7)//check layer is not empty
		{
			while(1)
			{
				LAYER += 0x5;

				GraphCurve curve;

				file.seekg(LAYER + 0x4, ios_base::beg);
				file >> w;

				pair<string, string> column = findDataByIndex(w-1);
				short nColY = w;
				if(column.first.size() > 0)
				{
					BOOST_LOG_(1, format("			graphs %d layer %d curve %d Y : %s.%s") % graphs.size() % graphs.back().layers.size() % graphs.back().layers.back().curves.size() % column.first.c_str() % column.second.c_str());
					curve.dataName = column.first;
					curve.yColumnName = column.second;
				}

				file.seekg(LAYER + 0x23, ios_base::beg);
				file >> w;

				column = findDataByIndex(w-1);
				if(column.first.size() > 0)
				{
					BOOST_LOG_(1, format("			graphs %d layer %d curve %d X : %s.%s") % graphs.size() % graphs.back().layers.size() % graphs.back().layers.back().curves.size() % column.first.c_str() % column.second.c_str());
					curve.xColumnName = column.second;
					if(curve.dataName != column.first)
					{
						BOOST_LOG_(1, format("			graphs %d X and Y from different tables") % graphs.size());
					}
				}

				file.seekg(LAYER + 0x4C, ios_base::beg);
				file >> curve.type;

				file.seekg(LAYER + 0x11, ios_base::beg);
				file >> curve.lineConnect;
				file >> curve.lineStyle;

				file.seekg(LAYER + 0x15, ios_base::beg);
				file >> w;
				curve.lineWidth=(double)w/500.0;

				file.seekg(LAYER + 0x19, ios_base::beg);
				file >> w;
				curve.symbolSize=(double)w/500.0;

				file.seekg(LAYER + 0x1C, ios_base::beg);
				file >> h;
				curve.fillArea = (h==2);

				file.seekg(LAYER + 0x1E, ios_base::beg);
				file >> curve.fillAreaType;

				//text
				if(curve.type == GraphCurve::TextPlot)
				{
					file.seekg(LAYER + 0x13, ios_base::beg);
					file >> curve.text.rotation;
					curve.text.rotation /= 10;
					file >> curve.text.fontSize;

					file.seekg(LAYER + 0x19, ios_base::beg);
					file >> h;
					switch(h)
					{
					case 26:
						curve.text.justify = TextProperties::Center;
						break;
					case 2:
						curve.text.justify = TextProperties::Right;
					    break;
					default:
						curve.text.justify = TextProperties::Left;
					    break;
					}

					file >> h;
					curve.text.fontUnderline = (h & 0x1);
					curve.text.fontItalic = (h & 0x2);
					curve.text.fontBold = (h & 0x8);
					curve.text.whiteOut = (h & 0x20);

					char offset;
					file.seekg(LAYER + 0x37, ios_base::beg);
					file >> offset;
					curve.text.xOffset = offset * 5;
					file >> offset;
					curve.text.yOffset = offset * 5;
				}

				//vector
				if(curve.type == GraphCurve::FlowVector || curve.type == GraphCurve::Vector)
				{
					file.seekg(LAYER + 0x56, ios_base::beg);
					file >> curve.vector.multiplier;

					file.seekg(LAYER + 0x5E, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
					{
						curve.vector.endXColumnName = column.second;
					}

					file.seekg(LAYER + 0x62, ios_base::beg);
					file >> h;

					column = findDataByIndex(nColY - 1 + h - 0x64);
					if(column.first.size() > 0)
					{
						curve.vector.endYColumnName = column.second;
					}

					file.seekg(LAYER + 0x18, ios_base::beg);
					file >> h;

					if(h >= 0x64)
					{
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.angleColumnName = column.second;
					}
					else if(h <= 0x08)
					{
						curve.vector.constAngle = 45*h;
					}

					file >> h;

					if(h >= 0x64 && h < 0x1F4)
					{
						column = findDataByIndex(nColY - 1 + h - 0x64);
						if(column.first.size() > 0)
							curve.vector.magnitudeColumnName = column.second;
					}
					else
					{
						curve.vector.constMagnitude = (int)curve.symbolSize;
					}

					file.seekg(LAYER + 0x66, ios_base::beg);
					file >> curve.vector.arrowLenght;
					file >> curve.vector.arrowAngle;

					file >> h;
					curve.vector.arrowClosed = !(h & 0x1);

					file >> w;
					curve.vector.width=(double)w/500.0;

					file.seekg(LAYER + 0x142, ios_base::beg);
					file >> h;
					switch(h)
					{
					case 2:
						curve.vector.position = VectorProperties::Midpoint;
						break;
					case 4:
						curve.vector.position = VectorProperties::Head;
						break;
					default:
						curve.vector.position = VectorProperties::Tail;
						break;
					}

				}

				//pie
				if(curve.type == GraphCurve::Pie)
				{
					file.seekg(LAYER + 0x92, ios_base::beg);
					file >> h;

					curve.pie.formatPercentages = (h&0x01);
					curve.pie.formatValues = (h&0x02);
					curve.pie.positionAssociate = (h&0x08);
					curve.pie.clockwiseRotation = (h&0x20);
					curve.pie.formatCategories = (h&0x80);

					file >> curve.pie.formatAutomatic;
					file >> curve.pie.distance;
					file >> curve.pie.viewAngle;

					file.seekg(LAYER + 0x98, ios_base::beg);
					file >> curve.pie.thickness;

					file.seekg(LAYER + 0x9A, ios_base::beg);
					file >> curve.pie.rotation;

					file.seekg(LAYER + 0x9E, ios_base::beg);
					file >> curve.pie.displacement;

					file.seekg(LAYER + 0xA0, ios_base::beg);
					file >> curve.pie.radius;
					file >> curve.pie.horizontalOffset;

					file.seekg(LAYER + 0xA6, ios_base::beg);
					file >> curve.pie.displacedSectionCount;
				}

				file.seekg(LAYER + 0xC2, ios_base::beg);
				file >> curve.fillAreaColor;
				file >> curve.fillAreaFirstColor;

				file.seekg(LAYER + 0xC6, ios_base::beg);
				file >> w;
				curve.fillAreaPatternWidth=(double)w/500.0;

				file.seekg(LAYER + 0xCA, ios_base::beg);
				file >> curve.fillAreaPatternColor;

				file.seekg(LAYER + 0xCE, ios_base::beg);
				file >> curve.fillAreaPattern;
				file >> curve.fillAreaPatternBorderStyle;
				file >> w;
				curve.fillAreaPatternBorderWidth=(double)w/500.0;
				file >> curve.fillAreaPatternBorderColor;

				file.seekg(LAYER + 0x16A, ios_base::beg);
				file >> curve.lineColor;
				curve.text.color = curve.lineColor;

				file.seekg(LAYER + 0x17, ios_base::beg);
				file >> curve.symbolType;

				file.seekg(LAYER + 0x12E, ios_base::beg);
				file >> curve.symbolFillColor;

				file.seekg(LAYER + 0x132, ios_base::beg);
				file >> curve.symbolColor;
				curve.vector.color = curve.symbolColor;

				file.seekg(LAYER + 0x136, ios_base::beg);
				file >> h;
				curve.symbolThickness = (h == 255 ? 1 : h);
				file >> curve.pointOffset;

				graphs.back().layers.back().curves.push_back(curve);

				LAYER += 0x1E7 + 0x1;

				int size;
				file.seekg(LAYER, ios_base::beg);
				file >> size;

				LAYER += size + (size > 0 ? 0x1 : 0) + 0x5;
	
				file.seekg(LAYER, ios_base::beg);
				file >> size;

				if(size != 0x1E7)
					break;
			}

		}
		//LAYER+=0x5*0x5+0x1ED*0x12;
		//LAYER+=2*0x5;

		LAYER += 0x5;
		//read axis breaks
		while(1)
		{
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			if(size == 0x2D)
			{
				LAYER += 0x5;
				file.seekg(LAYER + 2, ios_base::beg);
				file >> h;

				if(h == 2)
				{
					graphs.back().layers.back().xAxisBreak.minorTicksBefore = graphs.back().layers.back().xAxis.minorTicks;
					graphs.back().layers.back().xAxisBreak.scaleIncrementBefore = graphs.back().layers.back().xAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(graphs.back().layers.back().xAxisBreak);
				}
				else if(h == 4)
				{
					graphs.back().layers.back().yAxisBreak.minorTicksBefore = graphs.back().layers.back().yAxis.minorTicks;
					graphs.back().layers.back().yAxisBreak.scaleIncrementBefore = graphs.back().layers.back().yAxis.step;
					file.seekg(LAYER, ios_base::beg);
					readGraphAxisBreakInfo(graphs.back().layers.back().yAxisBreak);
				}
				LAYER += 0x2D + 0x1;
			}
			else
				break;
		}
		LAYER += 0x5;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().xAxis.minorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().xAxis.majorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().xAxis.tickAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().xAxis.formatAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().xAxis.tickAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().xAxis.formatAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;


		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().yAxis.minorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphGridInfo(graphs.back().layers.back().yAxis.majorGrid);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().yAxis.tickAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().yAxis.formatAxis[0]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisTickLabelsInfo(graphs.back().layers.back().yAxis.tickAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x5;
		file.seekg(LAYER, ios_base::beg);
		readGraphAxisFormatInfo(graphs.back().layers.back().yAxis.formatAxis[1]);
		LAYER += 0x1E7 + 1;

		LAYER += 0x2*0x5 + 0x1ED*0x6;

		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;
	}

	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin750Parser::skipObjectInfo()
{
	unsigned int POS = file.tellg();

	unsigned int size;
	file >> size;
	
	POS += 5;

	unsigned int LAYER = POS;
	LAYER += size + 0x1;
	while(1)// multilayer loop
	{
		// LAYER section
		LAYER +=0x5/* length of block = 0x12D + '\n'*/ + 0x12D + 0x1;
		//now structure is next : section_header_size=0x6F(4 bytes) + '\n' + section_header(0x6F bytes) + section_body_1_size(4 bytes) + '\n' + section_body_1 + section_body_2_size(maybe=0)(4 bytes) + '\n' + section_body_2 + '\n'
		//possible sections: column formulas, __WIPR, __WIOTN, __LayerInfoStorage
		//section name(column name in formula case) starts with 0x46 position
		while(1)
		{
			//section_header_size=0x6F(4 bytes) + '\n'
			LAYER += 0x5;

			//section_header
			string sec_name(41, 0);
			file.seekg(LAYER + 0x46, ios_base::beg);
			file >> sec_name;

			//section_body_1_size
			LAYER += 0x6F + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_1
			LAYER += 0x5;

			//section_body_2_size
			LAYER += size + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_2
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			//section_body_3_size
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			//section_body_3
			LAYER += 0x5;

			//close section 00 00 00 00 0A
			LAYER += size + (size > 0 ? 0x1 : 0);

			if(sec_name == "__LayerInfoStorage")
				break;

		}
		LAYER += 0x5;

		while(1)
		{
			LAYER += 0x5;

			LAYER += 0x1E7 + 0x1;
			file.seekg(LAYER, ios_base::beg);
			file >> size;

			LAYER += 0x5 + size + (size > 0 ? 0x1 : 0);

			file.seekg(LAYER, ios_base::beg);
			file >> size;

			if(size != 0x1E7)
				break;
		}

		LAYER += 0x5*0x5 + 0x1ED*0x12;
		file.seekg(LAYER, ios_base::beg);
		file >> size;

		if(size == 0)
			break;
	}
	file.seekg(LAYER + 0x5, ios_base::beg);
}

void Origin750Parser::readGraphGridInfo(GraphGrid &grid)
{
	unsigned int POS = file.tellg();
	
	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	grid.hidden = (h==0);

	file.seekg(POS + 0xF, ios_base::beg);
	file >> grid.color;

	file.seekg(POS + 0x12, ios_base::beg);
	file >> grid.style;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> w;
	grid.width = (double)w/500.0;
}

void Origin750Parser::readGraphAxisBreakInfo(GraphAxisBreak& axis_break)
{
	unsigned int POS = file.tellg();

	axis_break.show = true;

	file.seekg(POS + 0xB, ios_base::beg);
	file >> axis_break.from;

	file >> axis_break.to;

	file >> axis_break.scaleIncrementAfter;
	
	file >> axis_break.position;

	unsigned char h;
	file >> h;
	axis_break.log10 = (h==1);

	file >> axis_break.minorTicksAfter;
}

void Origin750Parser::readGraphAxisFormatInfo(GraphAxisFormat& format)
{
	unsigned int POS = file.tellg();

	unsigned char h;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	format.hidden = (h == 0);

	file.seekg(POS + 0xF, ios_base::beg);
	file >> format.color;

	file.seekg(POS + 0x4A, ios_base::beg);
	file >> w;
	format.majorTickLength = (double)w/10.0;

	file.seekg(POS + 0x15, ios_base::beg);
	file >> w;
	format.thickness = (double)w/500.0;

	file.seekg(POS + 0x25, ios_base::beg);
	file >> h;

	format.minorTicksType = (h>>6);
	format.majorTicksType = ((h>>4) & 3);
	format.axisPosition = (h & 0xF);
	switch(format.axisPosition) // need for testing
	{
	case 1:
		file.seekg(POS + 0x37, ios_base::beg);
		file >> h;
		format.axisPositionValue = (double)h;
		break;
	case 2:
		file.seekg(POS + 0x2F, ios_base::beg);
		file >> format.axisPositionValue;
		break;
	}
}

void Origin750Parser::readGraphAxisTickLabelsInfo(GraphAxisTick& tick)
{
	unsigned int POS = file.tellg();

	unsigned char h;
	unsigned char h1;
	short w;

	file.seekg(POS + 0x26, ios_base::beg);
	file >> h;
	tick.hidden = (h == 0);

	file.seekg(POS + 0xF, ios_base::beg);
	file >> tick.color;

	file.seekg(POS + 0x13, ios_base::beg);
	file >> w;
	tick.rotation = w/10;

	file >> tick.fontSize;

	file.seekg(POS + 0x1A, ios_base::beg);
	file >> h;
	tick.fontBold = (h & 0x8);
	
	file.seekg(POS + 0x23, ios_base::beg);
	file >> w;
	file >> h;
	file >> h1;
	tick.valueType = (ValueType)(h & 0xF);

	pair<string, string> column;
	switch(tick.valueType)
	{
	case Numeric:

		/*switch((h>>4))
		{
		case 0x9:
		tick.valueTypeSpecification=1;
		break;
		case 0xA:
		tick.valueTypeSpecification=2;
		break;
		case 0xB:
		tick.valueTypeSpecification=3;
		break;
		default:
		tick.valueTypeSpecification=0;
		}*/
		if((h>>4) > 7)
		{
			tick.valueTypeSpecification = (h>>4) - 8;
			tick.decimalPlaces = h1 - 0x40;
		}
		else
		{
			tick.valueTypeSpecification = (h>>4);
			tick.decimalPlaces = -1;
		}

		break;
	case Time:
	case Date:
	case Month:
	case Day:
	case ColumnHeading:
		tick.valueTypeSpecification = h1 - 0x40;
		break;
	case Text:
	case TickIndexedDataset:
	case Categorical:
		column = findDataByIndex(w-1);
		if(column.first.size() > 0)
		{
			tick.dataName = column.first;
			tick.columnName = column.second;
		}
		break;
	default: // Numeric Decimal 1.000
		tick.valueType = Numeric;
		tick.valueTypeSpecification = 0;
		break;
	}
}

void Origin750Parser::readProjectTree()
{
	readProjectTreeFolder(projectTree.begin());

	BOOST_LOG_(1, "Origin project Tree");
	for(tree<ProjectNode>::iterator it = projectTree.begin(projectTree.begin()); it != projectTree.end(projectTree.begin()); ++it)
	{
		BOOST_LOG_(1, string(projectTree.depth(it) - 1, ' ') + (*it).name);
	}
}

void Origin750Parser::readProjectTreeFolder(tree<ProjectNode>::iterator parent)
{
	unsigned int POS = file.tellg();

	double creationDate, modificationDate;
	POS += 5;

	file.seekg(POS + 0x10, ios_base::beg);
	file >> creationDate;

	file >> modificationDate;

	POS += 0x20 + 1 + 5;
	unsigned int size;
	file.seekg(POS, ios_base::beg);
	file >> size;

	POS += 5;

	// read folder name
	string name(size, 0);
	file.seekg(POS, ios_base::beg);
	file >> name;

	tree<ProjectNode>::iterator current_folder = projectTree.append_child(parent, ProjectNode(name, ProjectNode::Folder, doubleToPosixTime(creationDate), doubleToPosixTime(modificationDate)));
	POS += size + 1 + 5 + 5;

	unsigned int objectcount;
	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	POS += 5 + 5;

	for(unsigned int i = 0; i < objectcount; ++i)
	{
		POS += 5;
		char c;
		file.seekg(POS + 0x2, ios_base::beg);
		file >> c;

		unsigned int objectID;
		file.seekg(POS + 0x4, ios_base::beg);
		file >> objectID;

		if(c == 0x10)
		{
			projectTree.append_child(current_folder, ProjectNode(notes[objectID].name, ProjectNode::Note));
		}
		else
		{
			pair<ProjectNode::NodeType, string> object = findObjectByIndex(objectID);
			projectTree.append_child(current_folder, ProjectNode(object.second, object.first));
		}

		POS += 8 + 1 + 5 + 5;
	}

	file.seekg(POS, ios_base::beg);
	file >> objectcount;

	file.seekg(1, ios_base::cur);
	for(unsigned int i = 0; i < objectcount; ++i)
		readProjectTreeFolder(current_folder);
}

void Origin750Parser::readWindowProperties(Window& window, unsigned int size)
{
	unsigned int POS = file.tellg();

	window.objectID = objectIndex;
	++objectIndex;

	file.seekg(POS + 0x1B, ios_base::beg);
	file.read(reinterpret_cast<char*>(&window.frameRect), sizeof(window.frameRect));

	char c;
	file.seekg(POS + 0x32, ios_base::beg);
	file >> c;

	if(c & 0x01)
		window.state = Window::Minimized;
	else if(c & 0x02)
		window.state = Window::Maximized;

	file.seekg(POS + 0x69, ios_base::beg);
	file >> c;

	if(c & 0x01)
		window.title = Window::Label;
	else if(c & 0x02)
		window.title = Window::Name;
	else
		window.title = Window::Both;

	window.hidden = (c & 0x08);
	if(window.hidden)
	{
		BOOST_LOG_(1, format("			WINDOW %d NAME : %s	is hidden") % objectIndex % window.name.c_str());
	}
	
	double creationDate, modificationDate;
	file.seekg(POS + 0x73, ios_base::beg);
	file >> creationDate;
	file >> modificationDate;
	window.creationDate = doubleToPosixTime(creationDate);
	window.modificationDate = doubleToPosixTime(modificationDate);

	if(size > 0xC3)
	{
		unsigned int labellen = 0;
		file.seekg(POS + 0xC3, ios_base::beg);
		file >> c;
		while(c != '@')
		{
			file >> c;
			++labellen;
		}
		if(labellen > 0)
		{
			file.seekg(POS + 0xC3, ios_base::beg);
			file >> window.label.assign(labellen, 0);
		}

		BOOST_LOG_(1, format("			WINDOW %d LABEL: %s") % objectIndex % window.label);
	}
}

OriginParser* createOrigin750Parser(const string& fileName)
{
	return new Origin750Parser(fileName);
}