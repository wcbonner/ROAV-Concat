// ROAV-Concat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
CWinApp theApp;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
CString FindEXEFromPath(const CString & csEXE)
{
	CString csFullPath;
	//PathFindOnPath();
	CFileFind finder;
	if (finder.FindFile(csEXE))
	{
		finder.FindNextFile();
		csFullPath = finder.GetFilePath();
		finder.Close();
	}
	else
	{
		TCHAR filename[MAX_PATH];
		unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
		// Get the file name that we are running from.
		GetModuleFileName(AfxGetResourceHandle(), filename, buffersize);
		PathRemoveFileSpec(filename);
		PathAppend(filename, csEXE);
		if (finder.FindFile(filename))
		{
			finder.FindNextFile();
			csFullPath = finder.GetFilePath();
			finder.Close();
		}
		else
		{
			CString csPATH;
			csPATH.GetEnvironmentVariable(_T("PATH"));
			int iStart = 0;
			CString csToken(csPATH.Tokenize(_T(";"), iStart));
			while (csToken != _T(""))
			{
				if (csToken.Right(1) != _T("\\"))
					csToken.AppendChar(_T('\\'));
				csToken.Append(csEXE);
				if (finder.FindFile(csToken))
				{
					finder.FindNextFile();
					csFullPath = finder.GetFilePath();
					finder.Close();
					break;
				}
				csToken = csPATH.Tokenize(_T(";"), iStart);
			}
		}
	}
	return(csFullPath);
}
/////////////////////////////////////////////////////////////////////////////
static const CString QuoteFileName(const CString & Original)
{
	CString csQuotedString(Original);
	if (csQuotedString.Find(_T(" ")) >= 0)
	{
		csQuotedString.Insert(0, _T('"'));
		csQuotedString.AppendChar(_T('"'));
	}
	return(csQuotedString);
}
/////////////////////////////////////////////////////////////////////////////
std::string timeToISO8601(const time_t & TheTime)
{
	std::ostringstream ISOTime;
	//time_t timer = TheTime;
	struct tm UTC;// = gmtime(&timer);
	if (0 == gmtime_s(&UTC, &TheTime))
		//if (UTC != NULL)
	{
		ISOTime.fill('0');
		if (!((UTC.tm_year == 70) && (UTC.tm_mon == 0) && (UTC.tm_mday == 1)))
		{
			ISOTime << UTC.tm_year + 1900 << "-";
			ISOTime.width(2);
			ISOTime << UTC.tm_mon + 1 << "-";
			ISOTime.width(2);
			ISOTime << UTC.tm_mday << "T";
		}
		ISOTime.width(2);
		ISOTime << UTC.tm_hour << ":";
		ISOTime.width(2);
		ISOTime << UTC.tm_min << ":";
		ISOTime.width(2);
		ISOTime << UTC.tm_sec;
	}
	return(ISOTime.str());
}
std::wstring getTimeISO8601(void)
{
	time_t timer;
	time(&timer);
	std::string isostring(timeToISO8601(timer));
	std::wstring rval;
	rval.assign(isostring.begin(), isostring.end());

	return(rval);
}
time_t ISO8601totime(const std::string & ISOTime)
{
	struct tm UTC;
	UTC.tm_year = atol(ISOTime.substr(0, 4).c_str()) - 1900;
	UTC.tm_mon = atol(ISOTime.substr(5, 2).c_str()) - 1;
	UTC.tm_mday = atol(ISOTime.substr(8, 2).c_str());
	UTC.tm_hour = atol(ISOTime.substr(11, 2).c_str());
	UTC.tm_min = atol(ISOTime.substr(14, 2).c_str());
	UTC.tm_sec = atol(ISOTime.substr(17, 2).c_str());
#ifdef _MSC_VER
	_tzset();
	_get_daylight(&(UTC.tm_isdst));
#endif
	time_t timer = mktime(&UTC);
#ifdef _MSC_VER
	long Timezone_seconds = 0;
	_get_timezone(&Timezone_seconds);
	timer -= Timezone_seconds;
	int DST_hours = 0;
	_get_daylight(&DST_hours);
	long DST_seconds = 0;
	_get_dstbias(&DST_seconds);
	timer += DST_hours * DST_seconds;
#endif
	return(timer);
}
std::string timeToExcelDate(const time_t & TheTime)
{
	std::ostringstream ISOTime;
	time_t timer = TheTime;
	struct tm * UTC = gmtime(&timer);
	if (UTC != NULL)
	{
		ISOTime.fill('0');
		ISOTime << UTC->tm_year + 1900 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mon + 1 << "-";
		ISOTime.width(2);
		ISOTime << UTC->tm_mday << " ";
		ISOTime.width(2);
		ISOTime << UTC->tm_hour << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_min << ":";
		ISOTime.width(2);
		ISOTime << UTC->tm_sec;
	}
	return(ISOTime.str());
}
/////////////////////////////////////////////////////////////////////////////
typedef enum {
	GPS_FIX_UNKNOWN = 0,
	GPS_FIX_2D,
	GPS_FIX_3D
} GPS_FIX_TYPE;
#define GPS_MAX_SATELLITES 12
#define GPS_VALID_UTC_TIME 0x00000001	// If set, the stUTCTime field is valid.
#define GPS_VALID_LATITUDE 0x00000002	// If set, the dblLatitude field is valid.
#define GPS_VALID_LONGITUDE 0x00000004	// If set, the dblLongitude field is valid.
#define GPS_VALID_SPEED 0x00000008		// If set, the flSpeed field is valid.
#define GPS_VALID_HEADING 0x00000010	// If set, the flHeading field is valid.
#define GPS_VALID_MAGNETIC_VARIATION 0x00000020	// If set, the dblMagneticVariation field is valid.
#define GPS_VALID_ALTITUDE_WRT_SEA_LEVEL 0x00000040	// If set, the flAltitudeWRTSeaLevel field is valid.
#define GPS_VALID_ALTITUDE_WRT_ELLIPSOID 0x00000080	// If set, the flAltitudeWRTEllipsoid field is valid.
#define GPS_VALID_POSITION_DILUTION_OF_PRECISION 0x00000100	// If set, the flPositionDilutionOfPrecision field is valid.
#define GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION 0x00000200	// If set, the flHorizontalDilutionOfPrecision field is valid.
#define GPS_VALID_VERTICAL_DILUTION_OF_PRECISION 0x00000400	// If set, the flVerticalDilutionOfPrecision field is valid.
#define GPS_VALID_SATELLITE_COUNT 0x00000800	// If set, the dwSatelliteCount field is valid.
#define GPS_VALID_SATELLITES_USED_PRNS 0x00001000	// If set, the rgdwSatellitesUsedPRNs field is valid.
#define GPS_VALID_SATELLITES_IN_VIEW 0x00002000	// If set, the dwSatellitesInView field is valid.
#define GPS_VALID_SATELLITES_IN_VIEW_PRNS 0x00004000	// If set, the rgdwSatellitesInViewPRNs field is valid.
#define GPS_VALID_SATELLITES_IN_VIEW_ELEVATION 0x00008000	// If set, the rgdwSatellitesInViewElevation field is valid.
#define GPS_VALID_SATELLITES_IN_VIEW_AZIMUTH 0x00010000	// If set, the rgdwSatellitesInViewAzimuth field is valid.
#define GPS_VALID_SATELLITES_IN_VIEW_SIGNAL_TO_NOISE_RATIO 0x00020000	// If set, the rgdwSatellitesInViewSignalToNoiseRatio field is valid.
void csvline_populate(std::vector<std::string> &record, const std::string& line, char delimiter)
{
	int linepos = 0;
	int inquotes = false;
	int linemax = line.length();
	std::string curstring;
	record.clear();

	while (line[linepos] != 0 && linepos < linemax)
	{
		char c = line[linepos];
		if (!inquotes && curstring.length() == 0 && c == '"')
		{
			//beginquotechar
			inquotes = true;
		}
		else if (inquotes && c == '"')
		{
			//quotechar
			if ((linepos + 1 < linemax) && (line[linepos + 1] == '"'))
			{
				//encountered 2 double quotes in a row (resolves to 1 double quote)
				curstring.push_back(c);
				linepos++;
			}
			else
			{
				//endquotechar
				inquotes = false;
			}
		}
		else if (!inquotes && c == delimiter)
		{
			//end of field
			record.push_back(curstring);
			curstring = "";
		}
		else if (!inquotes && (c == '\r' || c == '\n'))
		{
			record.push_back(curstring);
			return;
		}
		else
		{
			curstring.push_back(c);
		}
		linepos++;
	}
	record.push_back(curstring);
	return;
}
class ROAV_GPS_POSITION {
public:
	ROAV_GPS_POSITION()
	{
		dblLatitude = 0;
		dblLongitude = 0;
		FixType = GPS_FIX_UNKNOWN;
		dwSatelliteCount = 0;
		flAltitudeWRTSeaLevel = 0;
		flSpeed = 0;
		flHeading = 0;
		AccelerometerX = 0;
		AccelerometerY = 0;
		AccelerometerZ = 0;
		dwValidFields = 0;
	};
	void Set(const std::string & line);
	std::string GetTXTHeader(void);
	std::string GetTXT(void);
	CTime stUTCTime;
	double dblLatitude;
	double dblLongitude;
	GPS_FIX_TYPE FixType;
	DWORD dwSatelliteCount;
	float flAltitudeWRTSeaLevel;
	float flSpeed;
	float flHeading;
	float AccelerometerX;
	float AccelerometerY;
	float AccelerometerZ;
	DWORD dwValidFields;
};
void ROAV_GPS_POSITION::Set(const std::string & line)
{
	std::vector<std::string> row;
	csvline_populate(row, line, ',');
	if (row.size() == 11)
	{
		if (!row[3].empty()) // I moved this to the font because I want to use it for updating the valid mask that I pay attention to when creating the KML
		{
			std::stringstream ss(row[3]);
			int LocalFixType = GPS_FIX_TYPE::GPS_FIX_UNKNOWN;
			ss >> LocalFixType;
			switch (LocalFixType)
			{
			case GPS_FIX_TYPE::GPS_FIX_2D:
				FixType = GPS_FIX_TYPE::GPS_FIX_2D;
				break;
			case GPS_FIX_TYPE::GPS_FIX_3D:
				FixType = GPS_FIX_TYPE::GPS_FIX_3D;
				break;
			default:
				break;
			}
		}
		if (!row[0].empty())
		{
			int Year = 0;
			std::stringstream ss(row[0].substr(0, 4)); // 20180620_11:06:42.000
			ss >> Year;
			ss = std::stringstream(row[0].substr(4, 2));
			int Month = 0;
			ss >> Month;
			int Day = 0;
			ss = std::stringstream(row[0].substr(6, 2));
			ss >> Day;
			int Hour = 0;
			ss = std::stringstream(row[0].substr(9, 2));
			ss >> Hour;
			int Minute = 0;
			ss = std::stringstream(row[0].substr(12, 2));
			ss >> Minute;
			int Second = 0;
			ss = std::stringstream(row[0].substr(15, 2));
			ss >> Second;
			stUTCTime = CTime(Year, Month, Day, Hour, Minute, Second);
			dwValidFields |= GPS_VALID_UTC_TIME;
		}
		if (!row[1].empty())
		{
			std::stringstream ss(row[1]);
			ss >> dblLatitude;
			if ((dblLatitude != 0) && (FixType != GPS_FIX_TYPE::GPS_FIX_UNKNOWN))
				dwValidFields |= GPS_VALID_LATITUDE;
		}
		if (!row[2].empty())
		{
			std::stringstream ss(row[2]);
			ss >> dblLongitude;
			if ((dblLongitude != 0) && (FixType != GPS_FIX_TYPE::GPS_FIX_UNKNOWN))
				dwValidFields |= GPS_VALID_LONGITUDE;
		}
		if (!row[4].empty())
		{
			std::stringstream ss(row[4]);
			ss >> dwSatelliteCount;
			dwValidFields |= GPS_VALID_SATELLITE_COUNT;
		}
		if (!row[5].empty())
		{
			std::stringstream ss(row[5]);
			ss >> flAltitudeWRTSeaLevel;
			if (FixType != GPS_FIX_TYPE::GPS_FIX_UNKNOWN)
				dwValidFields |= GPS_VALID_ALTITUDE_WRT_SEA_LEVEL;
		}
		if (!row[6].empty())
		{
			std::stringstream ss(row[6]);
			ss >> flSpeed;
			if (FixType != GPS_FIX_TYPE::GPS_FIX_UNKNOWN)
				dwValidFields |= GPS_VALID_SPEED;
		}
		if (!row[7].empty())
		{
			std::stringstream ss(row[7]);
			ss >> flHeading;
			if (FixType != GPS_FIX_TYPE::GPS_FIX_UNKNOWN)
				dwValidFields |= GPS_VALID_HEADING;
		}
		if (!row[8].empty())
		{
			std::stringstream ss(row[8]);
			ss >> AccelerometerX;
		}
		if (!row[9].empty())
		{
			std::stringstream ss(row[9]);
			ss >> AccelerometerY;
		}
		if (!row[10].empty())
		{
			std::stringstream ss(row[10]);
			ss >> AccelerometerZ;
		}
	}
}
std::string ROAV_GPS_POSITION::GetTXTHeader(void)
{
	std::string rval("Datetime\tLatitude\tLongitude\tFixType\tSatCount\tAltitude\tSpeedKph\tHeading\tAccelerometerX\tAccelerometerY\tAccelerometerZ");
	return(rval);
}

std::string ROAV_GPS_POSITION::GetTXT(void)
{
	std::stringstream ss;
	ss << timeToExcelDate(stUTCTime.GetTime());
	ss << std::setprecision(9) << "\t" << dblLatitude << "\t" << dblLongitude << std::setprecision(3) << "\t" << FixType << "\t" << dwSatelliteCount << "\t" << flAltitudeWRTSeaLevel << "\t" << flSpeed << "\t" << flHeading << "\t" << AccelerometerX << "\t" << AccelerometerY << "\t" << AccelerometerZ;
	std::string rval(ss.str());
	return(rval);
}
#ifndef M_PI
	#define M_PI       3.14159265358979323846L
#endif
inline double deg2rad(double Deg) { return(Deg * M_PI / 180.0); }
inline double rad2deg(double Rad) { return(Rad * 180.0 / M_PI); }
int OutputKML(CString & csKML,
	vector<ROAV_GPS_POSITION> & m_gpsLog,
	vector<ROAV_GPS_POSITION> & m_WayPoints,
	CString csName = _T("ROAV_Concat"))
{
	TIME_ZONE_INFORMATION tzInfo;
	DWORD nDST = GetTimeZoneInformation(&tzInfo);
	CTimeSpan tsBias(0, 0, tzInfo.Bias, 0);
	switch (nDST)
	{
	case TIME_ZONE_ID_STANDARD:
		tsBias += CTimeSpan(0, 0, tzInfo.StandardBias, 0);
		break;
	case TIME_ZONE_ID_DAYLIGHT:
		tsBias += CTimeSpan(0, 0, tzInfo.DaylightBias, 0);
		break;
	}
	int LastPathSeperator = csName.ReverseFind(_T('.'));
	if (LastPathSeperator > 0)
		csName = csName.Left(LastPathSeperator);

	const int XMLDataBuffSize = 1024;
	char* XMLDataBuff = new char[XMLDataBuffSize];
	XMLDataBuff[0] = 0;
	CComPtr<IStream> spMemoryStream(::SHCreateMemStream(NULL, 0));
	CComPtr<IXmlWriter> pWriter;
	if (SUCCEEDED(CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL)))
	{
		pWriter->SetOutput(spMemoryStream);
		pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);
		pWriter->WriteStartDocument(XmlStandalone_Omit);
		pWriter->WriteStartElement(NULL, _T("kml"), _T("http://www.opengis.net/kml/2.2"));
		pWriter->WriteAttributeString(_T("xmlns"), _T("gx"), NULL, _T("http://www.google.com/kml/ext/2.2"));
		//pWriter->WriteAttributeString(_T("xmlns"), _T("kml"), NULL, _T("http://www.opengis.net/kml/2.2"));
		//pWriter->WriteAttributeString(_T("xmlns"), _T("atom"), NULL, _T("http://www.w3.org/2005/Atom"));
		pWriter->WriteStartElement(NULL, _T("Document"), NULL);
		pWriter->WriteElementString(NULL, _T("name"), NULL, csName.GetString());
		pWriter->WriteElementString(NULL, _T("open"), NULL, _T("1"));

		pWriter->WriteStartElement(NULL, _T("Style"), NULL);
		pWriter->WriteAttributeString(NULL, _T("id"), NULL, _T("LineStyleWim0"));
		pWriter->WriteStartElement(NULL, _T("LineStyle"), NULL);
		pWriter->WriteElementString(NULL, _T("color"), NULL, _T("99ffac59"));
		pWriter->WriteElementString(NULL, _T("width"), NULL, _T("6"));
		pWriter->WriteEndElement(); // LineStyle
		pWriter->WriteEndElement(); // Style

		pWriter->WriteStartElement(NULL, _T("Style"), NULL);
		pWriter->WriteAttributeString(NULL, _T("id"), NULL, _T("LineStyleWim1"));
		pWriter->WriteStartElement(NULL, _T("LineStyle"), NULL);
		pWriter->WriteElementString(NULL, _T("color"), NULL, _T("ff00ffff"));
		pWriter->WriteElementString(NULL, _T("width"), NULL, _T("6"));
		pWriter->WriteEndElement(); // LineStyle
		pWriter->WriteEndElement(); // Style

		pWriter->WriteStartElement(NULL, _T("StyleMap"), NULL);
		pWriter->WriteAttributeString(NULL, _T("id"), NULL, _T("LineStyleWim"));
		pWriter->WriteStartElement(NULL, _T("Pair"), NULL);
		pWriter->WriteElementString(NULL, _T("key"), NULL, _T("normal"));
		pWriter->WriteElementString(NULL, _T("styleUrl"), NULL, _T("#LineStyleWim0"));
		pWriter->WriteEndElement(); // Pair
		pWriter->WriteStartElement(NULL, _T("Pair"), NULL);
		pWriter->WriteElementString(NULL, _T("key"), NULL, _T("highlight"));
		pWriter->WriteElementString(NULL, _T("styleUrl"), NULL, _T("#LineStyleWim1"));
		pWriter->WriteEndElement(); // Pair
		pWriter->WriteEndElement(); // StyleMap

		auto pos = m_gpsLog.begin();
		while (pos != m_gpsLog.end())
		{
			pWriter->WriteStartElement(NULL, _T("Folder"), NULL);	// https://developers.google.com/kml/documentation/kmlreference#folder
			pWriter->WriteElementString(NULL, _T("name"), NULL, pos->stUTCTime.Format(_T("%Y-%m-%d")));
			pWriter->WriteElementString(NULL, _T("open"), NULL, _T("0"));

			pWriter->WriteStartElement(NULL, _T("Placemark"), NULL);	// https://developers.google.com/kml/documentation/kmlreference#placemark
			pWriter->WriteElementString(NULL, _T("styleUrl"), NULL, _T("#LineStyleWim"));
			pWriter->WriteStartElement(NULL, _T("LineString"), NULL); // https://developers.google.com/kml/documentation/kmlreference#linestring
			pWriter->WriteElementString(NULL, _T("visibility"), NULL, _T("1"));
			//pWriter->WriteElementString(NULL, _T("extrude"), NULL, _T("1"));
			pWriter->WriteElementString(NULL, _T("tessellate"), NULL, _T("1"));
			pWriter->WriteElementString(NULL, _T("altitudeMode"), NULL, _T("clampToGround "));
			pWriter->WriteStartElement(NULL, _T("coordinates"), NULL);

			double m_Speed = 0;
			double m_SpeedMax = 0;
			ROAV_GPS_POSITION m_SpeedMaxPos;
			ROAV_GPS_POSITION m_AltitudeMaxPos;
			ROAV_GPS_POSITION m_AltitudeMinPos;
			double m_Odometer = 0;
			BOOL m_bMPH = TRUE;
			m_AltitudeMinPos.flAltitudeWRTSeaLevel = FLT_MAX;

			auto lastpos = pos;
			while ((pos != m_gpsLog.end()) && (lastpos->stUTCTime.GetDay() == pos->stUTCTime.GetDay()))
			{
				if ((pos->dwSatelliteCount > 0) && (pos->dwValidFields & GPS_VALID_LATITUDE) && (pos->dwValidFields & GPS_VALID_LONGITUDE))
				{
					std::wstringstream ss;
					ss << std::setprecision(15) << " " << pos->dblLongitude << "," << pos->dblLatitude;
					pWriter->WriteString(ss.str().c_str());

					if (pos->dwValidFields & GPS_VALID_ALTITUDE_WRT_SEA_LEVEL)
					{
						if (m_AltitudeMaxPos.flAltitudeWRTSeaLevel < pos->flAltitudeWRTSeaLevel)
							m_AltitudeMaxPos = *pos;
						if (m_AltitudeMinPos.flAltitudeWRTSeaLevel > pos->flAltitudeWRTSeaLevel)
							m_AltitudeMinPos = *pos;
					}
					CTimeSpan TimeSinceUpdate = CTime(pos->stUTCTime) - CTime(lastpos->stUTCTime);
					double Hours = double(TimeSinceUpdate.GetTotalSeconds()) / 3600.0;
					m_Odometer += pos->flSpeed * Hours;
					if (m_bMPH == TRUE)
						m_Speed = 0.6213711922L * pos->flSpeed; // Pull Speed from what the GPS logged, not calculated from GPS Position Differences
					else
						m_Speed = pos->flSpeed; // Pull Speed from what the GPS logged, not calculated from GPS Position Differences
					if (m_Speed > m_SpeedMax)
					{
						m_SpeedMax = m_Speed;
						m_SpeedMaxPos = *pos;
					}
				}
				lastpos = pos;
				pos++;
			}
			pWriter->WriteEndElement(); // coordinates
			pWriter->WriteEndElement(); // LineString
			//pWriter->WriteElementString(NULL, _T("name"), NULL, _T("Path"));
			CString csFoo;
			csFoo.Format(_T("Distance %.1f %s"), m_bMPH ? 0.6213711922L * m_Odometer : m_Odometer, m_bMPH ? _T("miles") : _T("kilometers"));
			pWriter->WriteElementString(NULL, _T("name"), NULL, csFoo.GetString());
			pWriter->WriteEndElement(); // Placemark
			if (m_SpeedMax > 0)
			{
				pWriter->WriteStartElement(NULL, _T("Placemark"), NULL);
				csFoo.Format(_T("Max Speed %.1f %s"), m_SpeedMax, m_bMPH ? _T("mph") : _T("kph"));
				pWriter->WriteElementString(NULL, _T("name"), NULL, csFoo.GetString());
				pWriter->WriteElementString(NULL, _T("visibility"), NULL, _T("1"));
				pWriter->WriteStartElement(NULL, _T("TimeStamp"), NULL);
				CTime posTime(m_SpeedMaxPos.stUTCTime);
				posTime -= tsBias;
				pWriter->WriteElementString(NULL, _T("when"), NULL, posTime.FormatGmt(_T("%Y-%m-%dT%H:%M:%SZ")));
				pWriter->WriteEndElement(); // TimeStamp
				pWriter->WriteStartElement(NULL, _T("Point"), NULL);
				csFoo.Format(_T("%.15g,%.15g,%g"), m_SpeedMaxPos.dblLongitude, m_SpeedMaxPos.dblLatitude, double(m_SpeedMaxPos.flAltitudeWRTSeaLevel));
				pWriter->WriteElementString(NULL, _T("coordinates"), NULL, csFoo.GetString());
				pWriter->WriteEndElement(); // Point
				pWriter->WriteEndElement(); // Placemark
			}
			// Max Altitude
			if (m_AltitudeMaxPos.dwValidFields & GPS_VALID_ALTITUDE_WRT_SEA_LEVEL)
			{
				pWriter->WriteStartElement(NULL, _T("Placemark"), NULL);
				csFoo.Format(_T("Max Altitude %.1f ft"), double(m_AltitudeMaxPos.flAltitudeWRTSeaLevel) * 3.28083989501312);
				pWriter->WriteElementString(NULL, _T("name"), NULL, csFoo.GetString());
				pWriter->WriteElementString(NULL, _T("visibility"), NULL, _T("1"));
				pWriter->WriteStartElement(NULL, _T("TimeStamp"), NULL);
				CTime posTime(m_AltitudeMaxPos.stUTCTime);
				posTime -= tsBias;
				pWriter->WriteElementString(NULL, _T("when"), NULL, posTime.FormatGmt(_T("%Y-%m-%dT%H:%M:%SZ")));
				pWriter->WriteEndElement(); // TimeStamp
				pWriter->WriteStartElement(NULL, _T("Point"), NULL);
				csFoo.Format(_T("%.15g,%.15g,%g"), m_AltitudeMaxPos.dblLongitude, m_AltitudeMaxPos.dblLatitude, double(m_AltitudeMaxPos.flAltitudeWRTSeaLevel));
				pWriter->WriteElementString(NULL, _T("coordinates"), NULL, csFoo.GetString());
				pWriter->WriteEndElement(); // Point
				pWriter->WriteEndElement(); // Placemark
			}
			// Min Altitude
			if (m_AltitudeMinPos.dwValidFields & GPS_VALID_ALTITUDE_WRT_SEA_LEVEL)
			{
				pWriter->WriteStartElement(NULL, _T("Placemark"), NULL);
				csFoo.Format(_T("Min Altitude %.1f ft"), double(m_AltitudeMinPos.flAltitudeWRTSeaLevel) * 3.28083989501312);
				pWriter->WriteElementString(NULL, _T("name"), NULL, csFoo.GetString());
				pWriter->WriteElementString(NULL, _T("visibility"), NULL, _T("1"));
				pWriter->WriteStartElement(NULL, _T("TimeStamp"), NULL);
				CTime posTime(m_AltitudeMinPos.stUTCTime);
				posTime -= tsBias;
				pWriter->WriteElementString(NULL, _T("when"), NULL, posTime.FormatGmt(_T("%Y-%m-%dT%H:%M:%SZ")));
				pWriter->WriteEndElement(); // TimeStamp
				pWriter->WriteStartElement(NULL, _T("Point"), NULL);
				csFoo.Format(_T("%.15g,%.15g,%g"), m_AltitudeMinPos.dblLongitude, m_AltitudeMinPos.dblLatitude, double(m_AltitudeMinPos.flAltitudeWRTSeaLevel));
				pWriter->WriteElementString(NULL, _T("coordinates"), NULL, csFoo.GetString());
				pWriter->WriteEndElement(); // Point
				pWriter->WriteEndElement(); // Placemark
			}
			pWriter->WriteEndElement(); // Folder
		};
		pWriter->WriteFullEndElement(); // Document
		pWriter->WriteComment(L" Copyright © 2018 William C Bonner ");
		pWriter->WriteEndDocument();
		pWriter->Flush();
		// Allocates enough memeory for the xml content.
		STATSTG ssStreamData = { 0 };
		spMemoryStream->Stat(&ssStreamData, STATFLAG_NONAME);
		SIZE_T cbSize = ssStreamData.cbSize.LowPart;
		if (cbSize >= XMLDataBuffSize)
		{
			delete[] XMLDataBuff;
			XMLDataBuff = new char[cbSize + 1];
		}
		// Copies the content from the stream to the buffer.
		LARGE_INTEGER position;
		position.QuadPart = 0;
		spMemoryStream->Seek(position, STREAM_SEEK_SET, NULL);
		ULONG cbRead;
		spMemoryStream->Read(XMLDataBuff, cbSize, &cbRead);
		XMLDataBuff[cbSize] = '\0';
		csKML = CString(XMLDataBuff);
		delete[] XMLDataBuff;
	}
	return(csKML.GetLength());
}
int OutputTXT(CString & csTXT,
	vector<ROAV_GPS_POSITION> & m_gpsLog)
{
	csTXT = CString(m_gpsLog.begin()->GetTXTHeader().c_str());
	csTXT.Append(_T("\r\n"));
	for (auto pos1 = m_gpsLog.begin(); pos1 != m_gpsLog.end(); pos1++)
	{
		csTXT.Append(CString(pos1->GetTXT().c_str()));
		csTXT.Append(_T("\r\n"));
	}
	return(csTXT.GetLength());
}
/////////////////////////////////////////////////////////////////////////////
bool SplitROAVPath(
	CString csSrcPath,
	CString & DestDir,
	CString & DestFileName,
	CString & DestFileExt
)
{	
	bool rval = false;
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	errno_t err = _tsplitpath_s(csSrcPath.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	if (err == 0)
	{
		rval = true;
		DestDir = CString(drive) + CString(dir);
		DestFileName = CString(fname);
		DestFileExt = CString(ext);
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "version")
CString GetFileVersion(const CString & filename)
{
	CString rval;
	// get The Version number of the file
	DWORD dwVerHnd = 0;
	DWORD nVersionInfoSize = ::GetFileVersionInfoSize((LPTSTR)filename.GetString(), &dwVerHnd);
	if (nVersionInfoSize > 0)
	{
		UINT *puVersionLen = new UINT;
		LPVOID pVersionInfo = new char[nVersionInfoSize];
		BOOL bTest = ::GetFileVersionInfo((LPTSTR)filename.GetString(), dwVerHnd, nVersionInfoSize, pVersionInfo);
		// Pull out the version number
		if (bTest)
		{
			LPVOID pVersionNum = NULL;
			bTest = ::VerQueryValue(pVersionInfo, _T("\\"), &pVersionNum, puVersionLen);
			if (bTest)
			{
				DWORD dwFileVersionMS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionMS;
				DWORD dwFileVersionLS = ((VS_FIXEDFILEINFO *)pVersionNum)->dwFileVersionLS;
				rval.Format(_T("%d.%d.%d.%d"), HIWORD(dwFileVersionMS), LOWORD(dwFileVersionMS), HIWORD(dwFileVersionLS), LOWORD(dwFileVersionLS));
			}
		}
		delete puVersionLen;
		delete[] pVersionInfo;
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
CString GetFileVersionString(const CString & filename, const CString & string)
{
	CString rval;
	// get The Version number of the file
	DWORD dwVerHnd = 0;
	DWORD nVersionInfoSize = ::GetFileVersionInfoSize((LPTSTR)filename.GetString(), &dwVerHnd);
	if (nVersionInfoSize > 0)
	{
		UINT *puVersionLen = new UINT;
		LPVOID pVersionInfo = new char[nVersionInfoSize];
		BOOL bTest = ::GetFileVersionInfo((LPTSTR)filename.GetString(), dwVerHnd, nVersionInfoSize, pVersionInfo);
		if (bTest)
		{
			// Structure used to store enumerated languages and code pages.
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			}	*lpTranslate = NULL;
			unsigned int cbTranslate;
			// Read the list of languages and code pages.
			::VerQueryValue(pVersionInfo, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate);
			// Read the file description for each language and code page.
			LPVOID lpSubBlockValue = NULL;
			unsigned int SubBlockValueSize = 1;
			for (unsigned int i = 0; i < (cbTranslate / sizeof(struct LANGANDCODEPAGE)); i++)
			{
				CString SubBlockName;
				SubBlockName.Format(_T("\\StringFileInfo\\%04x%04x\\%s"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage, string.GetString());
				// Retrieve file description for language and code page "i". 
				::VerQueryValue(pVersionInfo, (LPTSTR)SubBlockName.GetString(), &lpSubBlockValue, &SubBlockValueSize);
			}
			if (SubBlockValueSize > 0)
				rval = CString((LPTSTR)lpSubBlockValue, SubBlockValueSize - 1);
		}
		delete puVersionLen;
		delete[] pVersionInfo;
	}
	return(rval);
}
/////////////////////////////////////////////////////////////////////////////
const CString GetLogFileName()
{
	static CString csLogFileName;
	if (csLogFileName.IsEmpty())
	{
		TCHAR szLogFilePath[MAX_PATH] = _T("");
		SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szLogFilePath);
		std::wostringstream woss;
		woss << AfxGetAppName();
		time_t timer;
		time(&timer);
		struct tm UTC;
		if (!gmtime_s(&UTC, &timer))
		{
			woss << "-";
			woss.fill('0');
			woss << UTC.tm_year + 1900 << "-";
			woss.width(2);
			woss << UTC.tm_mon + 1;
		}
		PathAppend(szLogFilePath, woss.str().c_str());
		PathAddExtension(szLogFilePath, _T(".txt"));
		csLogFileName = CString(szLogFilePath);
	}
	return(csLogFileName);
}
CTime ROAVFileNameToDate(const CString & FileA)
{
	CTime rval;
	CString Dir;
	CString FileName;
	CString FileExt;
	if (SplitROAVPath(FileA, Dir, FileName, FileExt))
	{
		// 2018_0909_220439_076A
		int year = 0;
		int month = 0;
		int day = 0;
		int hour = 0;
		int minute = 0;
		int second = 0;
		std::wstring ssFName(FileName.GetString());
		//std::wstringstream foo;
		//foo = std::wstringstream(ssFName.substr(0, 4));
		//foo >> year;
		std::wstringstream(ssFName.substr(0, 4)) >> year;
		std::wstringstream(ssFName.substr(5, 2)) >> month;
		std::wstringstream(ssFName.substr(7, 2)) >> day;
		std::wstringstream(ssFName.substr(10, 2)) >> hour;
		std::wstringstream(ssFName.substr(12, 2)) >> minute;
		std::wstringstream(ssFName.substr(14, 2)) >> second;
		rval = CTime(year, month, day, hour, minute, second);
	}
	return(rval);
}
bool ROAVFileSort(const CString & FileA, const CString & FileB)
{
	bool rVal = false;
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	errno_t err = _tsplitpath_s(FileA.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	if (err == 0)
	{
		CString FileAName(fname);
		err = _tsplitpath_s(FileB.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
		if (err == 0)
		{
			CString FileBName(fname);
			rVal = FileAName.Compare(FileBName) < 0;
		}
	}
	return(rVal);
}
/////////////////////////////////////////////////////////////////////////////
void CreateSourceFileList(const CString &csFirstFileName, const CString &csLastFileName, std::vector<CString> &SourceImageList, unsigned long long &CombinedSourceImageSize)
{
	// I need to parse the filenames, and recurse into the RO directory, and sort them properly.

	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113803_057.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113853_058A.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113855_059A.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_114158_060A.MP4"));

	// Need to check that both first and last exist.
	// Need to check that both first and last are in the same directory.
	// Build a list of everything in the directory.
	// Add all items from a RO subdirectory.
	// Sort the list. (because the RO directory name is longer, I need to split the filenames from the directory names and just sort those)
	// Remove any files in the list before our firstfilename
	// remove any files in the list after our lastfilename

	CString DestDir;
	CString DestFileName;
	CString DestFileExt;
	SplitROAVPath(csFirstFileName, DestDir, DestFileName, DestFileExt);
	//SplitImagePath(csLastFileName, DestParentDir, DirNumLast, DestChildSuffix, DestFilePrefix, DestFileNumDigits, FileNumLast, DestFileExt);

	CString csFinderString;
	csFinderString.Format(_T("%s*%s"), DestDir.GetString(), DestFileExt.GetString());
	CFileFind finder;
	bool bworking = finder.FindFile(csFinderString.GetString());
	while (bworking)
	{
		bworking = finder.FindNextFile();
		if (finder.GetLength() > 0)
		{
			SourceImageList.push_back(finder.GetFilePath());
			CombinedSourceImageSize += finder.GetLength();
		}
	}
	finder.Close();
	csFinderString.Format(_T("%sRO\\*%s"), DestDir.GetString(), DestFileExt.GetString());
	bworking = finder.FindFile(csFinderString.GetString());
	while (bworking)
	{
		bworking = finder.FindNextFile();
		if (finder.GetLength() > 0)
			SourceImageList.push_back(finder.GetFilePath());
	}
	finder.Close();

	std::sort(SourceImageList.begin(), SourceImageList.end(), ROAVFileSort);

	while (SourceImageList.back().Compare(csLastFileName) != 0)
		SourceImageList.pop_back();
	while (SourceImageList.front().Compare(csFirstFileName) != 0)
		SourceImageList.erase(SourceImageList.begin());

	for (auto SourceFile = SourceImageList.begin(); SourceFile != SourceImageList.end(); SourceFile++)
	{
		CFileStatus StatusVideo;
		if (TRUE == CFile::GetStatus(SourceFile->GetString(), StatusVideo))
			CombinedSourceImageSize += StatusVideo.m_size;
	}
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			if ((argc % 2 != 0) && (argc < 4))
			{
				std::wcout << "command Line Format:" << std::endl;
				std::wcout << "\t" << argv[0] << " VideoName [-no mp4][-no kmz][-preset [ultrafast|superfast|veryfast|faster|fast|medium|slow|slower|veryslow]] [-crf [0-28]] PathToFirstFile.mp4 PathToLastFile.mp4" << std::endl;
			}
			else
			{
				std::vector<CString> SourceImageList;
				unsigned long long CombinedSourceImageSize = 0;
				CString csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
				CString csVideoName(argv[1]);
				CString csH265preset(_T("veryfast"));	// h.265 option: Valid presets are ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo. The default is medium.
				CString csH265crf(_T("23"));			// use instead of default - crf 28 (Added 4 / 05 / 2018)
				bool bKMZOutput = true;
				bool bMP4Output = true;
				std::locale mylocale("");   // get global locale
				std::wcout.imbue(mylocale);  // imbue global locale
				for (auto argindex = 2; argindex < argc;)
				{
					CString csFirstFileName = CString(argv[argindex++]);
					CString csLastFileName = CString(argv[argindex++]);
					if (!csFirstFileName.Compare(_T("-preset")))
					{
						csH265preset = csLastFileName;
					}
					else if (!csFirstFileName.Compare(_T("-crf")))
					{
						csH265crf = csLastFileName;
					}
					else if (!csFirstFileName.CompareNoCase(_T("-no")))
					{
						if (!csLastFileName.CompareNoCase(_T("kmz")))
							bKMZOutput = false;
						else if (!csLastFileName.CompareNoCase(_T("mp4")))
							bMP4Output = false;
					}
					else
					{
						CreateSourceFileList(csFirstFileName, csLastFileName, SourceImageList, CombinedSourceImageSize);
						std::wcout << "[" << getTimeISO8601() << "] " << "Segment[" << (argindex / 2 - 1) << "]: " << csFirstFileName.GetString() << " " << csLastFileName.GetString() << std::endl;
					}
				}
				std::wcout << "[" << getTimeISO8601() << "] " << "First File: " << SourceImageList.front().GetString() << std::endl;
				std::wcout << "[" << getTimeISO8601() << "] " << "Last File:  " << SourceImageList.back().GetString() << std::endl;
				std::wcout << "[" << getTimeISO8601() << "] " << "Total Files: " << SourceImageList.size() << std::endl;
				std::wcout << "[" << getTimeISO8601() << "] " << "Combined Source Image Size: " << (CombinedSourceImageSize >> 20) << " MB" << std::endl;

				//std::string ssVideoLenghth(timeToISO8601(SourceImageList.size() * 2));
				//std::wstring wsVideoLenghth(ssVideoLenghth.begin(), ssVideoLenghth.end());
				//std::wcout << "[" << getTimeISO8601() << "] " << "Estimated Video Length: " << wsVideoLenghth << std::endl;

				std::wofstream m_LogFile(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
				if (m_LogFile.is_open())
				{
					TCHAR tcHostName[256] = TEXT("");
					DWORD dwSize = sizeof(tcHostName);
					GetComputerNameEx(ComputerNameDnsHostname, tcHostName, &dwSize);
					std::locale mylocale("");   // get global locale
					m_LogFile.imbue(mylocale);  // imbue global locale
					m_LogFile << "[" << getTimeISO8601() << "] LogFile Opened (" << tcHostName << ")" << std::endl;
					TCHAR filename[1024];
					unsigned long buffersize = sizeof(filename) / sizeof(TCHAR);
					// Get the file name that we are running from.
					GetModuleFileName(AfxGetResourceHandle(), filename, buffersize);
					m_LogFile << "[                   ] Program: " << CStringA(filename).GetString() << std::endl;
					m_LogFile << "[                   ] Version: " << CStringA(GetFileVersion(filename)).GetString() << " Built: " __DATE__ " at " __TIME__ << std::endl;
					m_LogFile << "[                   ] Command: ";
					for (auto index = 0; index < argc; index++) m_LogFile << QuoteFileName(argv[index]).GetString() << " "; m_LogFile << std::endl;
					m_LogFile << "[" << getTimeISO8601() << "] " << "First File: " << SourceImageList.front().GetString() << std::endl;
					m_LogFile << "[" << getTimeISO8601() << "] " << "Last File:  " << SourceImageList.back().GetString() << std::endl;
					m_LogFile << "[" << getTimeISO8601() << "] " << "Total Files: " << SourceImageList.size() << std::endl;
					m_LogFile << "[" << getTimeISO8601() << "] " << "Combined Source Image Size: " << (CombinedSourceImageSize >> 20) << " MB" << std::endl;
					m_LogFile.close();
				}

				TCHAR szTempFileName[MAX_PATH] = _T(""); // I'm declaring this here so that I can cleanup a tempfile at the end if I've created it.
				TCHAR szPath[MAX_PATH] = _T("");
				SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, 0, szPath);
				PathAddBackslash(szPath);
				CString csImageDirectory(szPath);
				csImageDirectory.Append(csVideoName);
				CString csVideoFullPath(csImageDirectory); csVideoFullPath.Append(_T(".mp4"));

				if (bKMZOutput)
				{
					vector<ROAV_GPS_POSITION> gpsLog, waypointLog;
					for (auto SourceFileName : SourceImageList)
					{
						TCHAR drive[_MAX_DRIVE];
						TCHAR dir[_MAX_DIR];
						TCHAR fname[_MAX_FNAME];
						TCHAR ext[_MAX_EXT];
						errno_t err = _tsplitpath_s(SourceFileName.GetString(), drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
						if (err == 0)
						{
							TCHAR path_buffer[_MAX_PATH];
							_tmakepath_s(path_buffer, _MAX_PATH, drive, dir, fname, _T("info"));
							std::ifstream InfoFile(path_buffer);
							if (InfoFile.is_open())
							{
								std::string line;
								while (getline(InfoFile, line) && InfoFile.good())
								{
									ROAV_GPS_POSITION pos;
									pos.Set(line);
									gpsLog.push_back(pos);
									if (!(gpsLog.size() % 600)) // update status for every ten minutes of gps points
										std::wcout << "\r[" << getTimeISO8601() << "] GPS Points: " << gpsLog.size();
								}
								InfoFile.close();
							}
						}
					}
					std::wcout << "\r[" << getTimeISO8601() << "] GPS Points: " << gpsLog.size() << std::endl;
					m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
					if (m_LogFile.is_open())
					{
						m_LogFile << "[" << getTimeISO8601() << "] GPS Points: " << gpsLog.size() << std::endl;
						m_LogFile.close();
					}

					CString csKMLFullPath(csImageDirectory); csKMLFullPath.Append(_T(".kmz"));
					std::ofstream KMLFile(csKMLFullPath.GetString(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
					if (KMLFile.is_open())
					{
						CString csKML;
						OutputKML(csKML, gpsLog, waypointLog, csVideoName);
						CString csName(csVideoName);
						csName.Append(_T(".kml"));
						CStringA csaOutPut(csKML);
						csKML.Empty();
						HZIP hz = CreateZip(0, 0x1000000, 0);
						ZipAdd(hz, csName.GetString(), csaOutPut.GetBuffer(), csaOutPut.GetLength());
						csaOutPut.Empty();
						void *zbuf;
						unsigned long zlen;
						ZipGetMemory(hz, &zbuf, &zlen);
						KMLFile.write((char *)zbuf, zlen);
						CloseZip(hz);
						KMLFile.close();
						try
						{
							CFileStatus StatusFirstVideo, StatusKMZFile;
							if (TRUE == CFile::GetStatus(SourceImageList.begin()->GetString(), StatusFirstVideo))
								if (TRUE == CFile::GetStatus(csKMLFullPath, StatusKMZFile))
								{
									std::wcout << "[" << getTimeISO8601() << "] Created KMZ file: " << csKMLFullPath.GetString() << ", File Size: " << (StatusKMZFile.m_size >> 10) << " KB" << std::endl;
									m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
									if (m_LogFile.is_open())
									{
										m_LogFile << "[" << getTimeISO8601() << "] Created KMZ file: " << csKMLFullPath.GetString() << ", File Size: " << (StatusKMZFile.m_size >> 10) << " KB" << std::endl;
										m_LogFile.close();
									}
									StatusKMZFile.m_ctime = StatusKMZFile.m_mtime = StatusFirstVideo.m_ctime;
									CFile::SetStatus(csKMLFullPath, StatusKMZFile);
								}
						}
						catch (CFileException *e)
						{
							TCHAR   szCause[512];
							e->GetErrorMessage(szCause, sizeof(szCause) / sizeof(TCHAR));
							CStringA csErrorMessage(szCause);
							csErrorMessage.Trim();
							std::wstringstream ss;
							ss << "[" << getTimeISO8601() << "] CFileException: (" << e->m_lOsError << ") " << csErrorMessage.GetString() << std::endl;
							TRACE(ss.str().c_str());
							m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
							if (m_LogFile.is_open())
							{
								m_LogFile << ss.str();
								m_LogFile.close();
							}
						}
					}
				}

				if (bMP4Output)
				{
					if (csFFMPEGPath.GetLength() > 0)
					{
						TCHAR szFontFilePath[MAX_PATH] = _T("");
						SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, szFontFilePath);
						PathAddBackslash(szFontFilePath);
						//PathAppend(szFontFilePath, _T("OCRAEXT"));
						PathAppend(szFontFilePath, _T("consola"));
						PathAddExtension(szFontFilePath, _T(".ttf"));
						CString csFontFilePath(szFontFilePath);
						csFontFilePath.Replace(_T("\\"), _T("/"));
						csFontFilePath.Replace(_T(":"), _T("\\\\:"));

						while (!SourceImageList.empty())
						{
							auto currentDay = ROAVFileNameToDate(SourceImageList.begin()->GetString()).GetDay();
							auto FileIterator = SourceImageList.begin();
							while (FileIterator != SourceImageList.end())
								if (ROAVFileNameToDate(FileIterator->GetString()).GetDay() == currentDay)
									FileIterator++;
								else
									break;
							std::vector<CString> SourceVideoList(SourceImageList.begin(), FileIterator);
							SourceImageList.erase(SourceImageList.begin(), FileIterator);

							csVideoFullPath = CString(szPath) + ROAVFileNameToDate(SourceVideoList.begin()->GetString()).Format(_T("%Y%m%d-")) + csVideoName + _T(".mp4");

							vector<wchar_t *> args;
							vector<wstring> mycommand;
							mycommand.push_back(csFFMPEGPath.GetString());
#ifdef _DEBUG
							mycommand.push_back(_T("-report"));
#else
							mycommand.push_back(_T("-hide_banner"));
#endif
							CString csFilterCommand;
							int OutFileIndex = 0;
							while (OutFileIndex < SourceVideoList.size())
								csFilterCommand.AppendFormat(_T("[%d:v]"), OutFileIndex++);
							csFilterCommand.AppendFormat(_T("concat=n=%d:v=1[v];"), OutFileIndex);
							csFilterCommand.Append(_T("[v]"));
							csFilterCommand.Append(_T("setpts=(1/60)*PTS"));
							csFilterCommand.Append(_T(",drawtext=fontfile="));
							csFilterCommand.Append(csFontFilePath);
							csFilterCommand.Append(_T(":fontcolor=white:fontsize=80:y=main_h-text_h-50:x=50:text=WimsWorld"));
							csFilterCommand.Append(_T("[o]"));
							if (csFilterCommand.GetLength() < 900) // Arbitrary number, seems to be a limitation in FFMPEG filter_complex command length
							{
								for (auto SourceFile = SourceVideoList.begin(); SourceFile != SourceVideoList.end(); SourceFile++)
								{
									mycommand.push_back(_T("-i")); mycommand.push_back(QuoteFileName(SourceFile->GetString()).GetString());
								}
								mycommand.push_back(_T("-filter_complex")); mycommand.push_back(csFilterCommand.GetString());
								mycommand.push_back(_T("-map")); mycommand.push_back(_T("[o]"));
							}
							else // The filter_complex command was too long, so I'll have to use the less flexible concat demuxer version that uses a temp file with list of input files.
							{
								// create temp file with one input file on each line
								// use -vf 
								//  Gets the temp path env string (no guarantee it's a valid path).
								TCHAR lpTempPathBuffer[MAX_PATH];
								auto dwRetVal = GetTempPath(MAX_PATH, lpTempPathBuffer);
								if (dwRetVal > MAX_PATH || (dwRetVal == 0))
								{
									std::wcout << "[" << getTimeISO8601() << "] " << "GetTempPath failed" << std::endl;
								}
								else
								{
									//  Generates a temporary file name. 
									if (0 == GetTempFileName(lpTempPathBuffer, _T("Wim"), 0, szTempFileName))
									{
										std::wcout << "[" << getTimeISO8601() << "] " << "GetTempFileName failed" << std::endl;
									}
									else
									{
										std::wofstream TmpFile(szTempFileName, std::ios_base::out | std::ios_base::app | std::ios_base::ate);
										if (TmpFile.is_open())
										{
											for (auto SourceFile = SourceVideoList.begin(); SourceFile != SourceVideoList.end(); SourceFile++)
											{
												CString TmpString(QuoteFileName(SourceFile->GetString()).GetString());
												TmpString.Replace(_T("\\"), _T("/"));
												TmpFile << _T("file ") << TmpString.GetString() << std::endl;
											}
											TmpFile.close();
											mycommand.push_back(_T("-f")); mycommand.push_back(_T("concat"));
											mycommand.push_back(_T("-safe")); mycommand.push_back(_T("0"));
											mycommand.push_back(_T("-i")); mycommand.push_back(szTempFileName);
											csFilterCommand.Empty();
											csFilterCommand.Append(_T("setpts=(1/60)*PTS"));
											csFilterCommand.Append(_T(",drawtext=fontfile="));
											csFilterCommand.Append(csFontFilePath);
											csFilterCommand.Append(_T(":fontcolor=white:fontsize=80:y=main_h-text_h-50:x=50:text=WimsWorld"));
											mycommand.push_back(_T("-vf")); mycommand.push_back(csFilterCommand.GetString());
											mycommand.push_back(_T("-an")); // No audio
										}
									}
								}
							}
							mycommand.push_back(_T("-c:v")); mycommand.push_back(_T("libx265"));		// use h.265 instead of default output for file extension (Added 3/26/2018)
							mycommand.push_back(_T("-crf")); mycommand.push_back(csH265crf.GetString());// use instead of default -crf 28 (Added 4/05/2018)
							mycommand.push_back(_T("-preset")); mycommand.push_back(csH265preset.GetString());	// h.265 option: Valid presets are ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo. The default is medium.
							// Some additions from https://support.google.com/youtube/answer/1722171?hl=en on 2014-08-24
							//mycommand.push_back(_T("--bframes")); mycommand.push_back(_T("2"));
							//mycommand.push_back(_T("--keyint")); mycommand.push_back(_T("15"));
							// And details from https://ffmpeg.zeranoe.com/forum/viewtopic.php?t=2318 on 2017-05-01
							// and https://sites.google.com/site/linuxencoding/x264-ffmpeg-mapping on 2017-05-01
							mycommand.push_back(_T("-movflags")); mycommand.push_back(_T("+faststart"));
							mycommand.push_back(_T("-bf")); mycommand.push_back(_T("2"));
							mycommand.push_back(_T("-g")); mycommand.push_back(_T("15"));
							mycommand.push_back(_T("-pix_fmt")); mycommand.push_back(_T("yuv420p"));
#ifdef _DEBUG
							mycommand.push_back(_T("-y"));
#else
							mycommand.push_back(_T("-n"));
#endif
							mycommand.push_back(QuoteFileName(csVideoFullPath).GetString());
							std::wcout << "[" << getTimeISO8601() << "]";

							m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
							if (m_LogFile.is_open())
							{
								m_LogFile << "[" << getTimeISO8601() << "] " << "First File:  " << SourceVideoList.front().GetString() << std::endl;
								m_LogFile << "[" << getTimeISO8601() << "] " << "Last File:   " << SourceVideoList.back().GetString() << std::endl;
								m_LogFile << "[" << getTimeISO8601() << "] " << "Total Files: " << SourceVideoList.size() << std::endl;
								m_LogFile << "[" << getTimeISO8601() << "]";
							}
							for (auto arg = mycommand.begin(); arg != mycommand.end(); arg++)
							{
								std::wcout << " " << *arg;
								if (m_LogFile.is_open())
									m_LogFile << " " << *arg;
								args.push_back((wchar_t *)arg->c_str());
							}
							std::wcout << std::endl;
							if (m_LogFile.is_open())
							{
								m_LogFile << std::endl;
								m_LogFile.close();
							}
							args.push_back(NULL);

							CTime ctStart(CTime::GetCurrentTime());
							CTimeSpan ctsTotal = CTime::GetCurrentTime() - ctStart;

							if (-1 == _tspawnvp(_P_WAIT, csFFMPEGPath.GetString(), &args[0])) //HACK: This really needs to be figured out, but I think it should work.
								std::wcout << "[" << getTimeISO8601() << "]  _tspawnvp failed: " /* << _sys_errlist[errno] */ << std::endl;

							ctsTotal = CTime::GetCurrentTime() - ctStart;
							auto TotalSeconds = ctsTotal.GetTotalSeconds();
							if (TotalSeconds > 0)
							{
								auto FPS = double(SourceVideoList.size()) / double(TotalSeconds);
								std::wcout << "[" << getTimeISO8601() << "] encoded " << SourceVideoList.size() << " frames in " << TotalSeconds << "s (" << FPS << "fps)" << std::endl;
								m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
								if (m_LogFile.is_open())
								{
									m_LogFile << "[" << getTimeISO8601() << "] encoded " << SourceVideoList.size() << " frames in " << TotalSeconds << "s (" << FPS << "fps)" << std::endl;
									m_LogFile.close();
								}
							}

							// If we created a temporary file, delete it.
							CFileStatus StatusTmpFile;
							if (TRUE == CFile::GetStatus(szTempFileName, StatusTmpFile))
							{
								std::wcout << "[" << getTimeISO8601() << "] Removing Temporary File: " << szTempFileName << std::endl;
								DeleteFile(szTempFileName);
							}

							try
							{
								CFileStatus StatusFirstVideo, StatusFinalVideo;
								if (TRUE == CFile::GetStatus(SourceVideoList.begin()->GetString(), StatusFirstVideo))
									if (TRUE == CFile::GetStatus(csVideoFullPath, StatusFinalVideo))
									{
										m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
										if (m_LogFile.is_open())
										{
											m_LogFile << "[" << getTimeISO8601() << "] Final File Size: " << (StatusFinalVideo.m_size >> 20) << " MB" << std::endl;
											m_LogFile.close();
										}
										StatusFinalVideo.m_ctime = StatusFinalVideo.m_mtime = StatusFirstVideo.m_ctime;
										CFile::SetStatus(csVideoFullPath, StatusFinalVideo);
									}
							}
							catch (CFileException *e)
							{
								TCHAR   szCause[512];
								e->GetErrorMessage(szCause, sizeof(szCause) / sizeof(TCHAR));
								CStringA csErrorMessage(szCause);
								csErrorMessage.Trim();
								std::wstringstream ss;
								ss << "[" << getTimeISO8601() << "] CFileException: (" << e->m_lOsError << ") " << csErrorMessage.GetString() << std::endl;
								TRACE(ss.str().c_str());
								m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
								if (m_LogFile.is_open())
								{
									m_LogFile << ss.str();
									m_LogFile.close();
								}
							}
						}
					}
				}
				m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
				if (m_LogFile.is_open())
				{
					m_LogFile << "[" << getTimeISO8601() << "] LogFile Closed" << std::endl;
					m_LogFile.close();
				}
			}
		}
	}
	else
	{
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}
	return nRetCode;
}
