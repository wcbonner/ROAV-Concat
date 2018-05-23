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
/////////////////////////////////////////////////////////////////////////////
bool SplitImagePath(
	CString csSrcPath,
	CString & DestParentDir,
	int & DestChildNum,
	CString & DestChildSuffix,
	CString & DestFilePrefix,
	int & DestFileNumDigits,
	int & DestFileNum,
	CString & DestFileExt
)
{
	bool rval = true;
	DestFileExt.Empty();
	while (csSrcPath[csSrcPath.GetLength() - 1] != _T('.'))
	{
		DestFileExt.Insert(0, csSrcPath[csSrcPath.GetLength() - 1]);
		csSrcPath.Truncate(csSrcPath.GetLength() - 1);
	}
	csSrcPath.Truncate(csSrcPath.GetLength() - 1); // get rid of dot

	CString csDestFileNum;
	DestFileNumDigits = 0;
	while (iswdigit(csSrcPath[csSrcPath.GetLength() - 1]))
	{
		csDestFileNum.Insert(0, csSrcPath[csSrcPath.GetLength() - 1]);
		DestFileNumDigits++;
		csSrcPath.Truncate(csSrcPath.GetLength() - 1);
	}
	DestFileNum = _wtoi(csDestFileNum.GetString());

	DestFilePrefix.Empty();
	while (iswalpha(csSrcPath[csSrcPath.GetLength() - 1]))
	{
		DestFilePrefix.Insert(0, csSrcPath[csSrcPath.GetLength() - 1]);
		csSrcPath.Truncate(csSrcPath.GetLength() - 1);
	}
	csSrcPath.Truncate(csSrcPath.GetLength() - 1); // get rid of backslash

	DestChildSuffix.Empty();
	while (iswalpha(csSrcPath[csSrcPath.GetLength() - 1]))
	{
		DestChildSuffix.Insert(0, csSrcPath[csSrcPath.GetLength() - 1]);
		csSrcPath.Truncate(csSrcPath.GetLength() - 1);
	}

	CString csDestChildNum;
	while (iswdigit(csSrcPath[csSrcPath.GetLength() - 1]))
	{
		csDestChildNum.Insert(0, csSrcPath[csSrcPath.GetLength() - 1]);
		csSrcPath.Truncate(csSrcPath.GetLength() - 1);
	}
	DestChildNum = _wtoi(csDestChildNum.GetString());

	DestParentDir = csSrcPath;
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
/////////////////////////////////////////////////////////////////////////////
void CreateSourceFileList(const CString &csFirstFileName, const CString &csLastFileName, std::vector<CString> &SourceImageList, unsigned long long &CombinedSourceImageSize)
{
	// I need to parse the filenames, and recurse into the RO directory, and sort them properly.

	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113803_057.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113853_058A.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_113855_059A.MP4"));
	//SourceImageList.push_back(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520_114158_060A.MP4"));

	//int DirNumFirst = 0;
	//int DirNumLast = 0;
	//int FileNumFirst = 0;
	//int FileNumLast = 0;
	//CString csFinderStringFormat;

	//CString DestParentDir;
	//CString DestChildSuffix;
	//CString DestFilePrefix;
	//CString DestFileExt;
	//int DestFileNumDigits;
	//SplitImagePath(csFirstFileName, DestParentDir, DirNumFirst, DestChildSuffix, DestFilePrefix, DestFileNumDigits, FileNumFirst, DestFileExt);
	//csFinderStringFormat.Format(_T("%s%%03d%s\\%s*.%s"), DestParentDir.GetString(), DestChildSuffix.GetString(), DestFilePrefix.GetString(), DestFileExt.GetString());
	//SplitImagePath(csLastFileName, DestParentDir, DirNumLast, DestChildSuffix, DestFilePrefix, DestFileNumDigits, FileNumLast, DestFileExt);

	//int DirNum = DirNumFirst;
	//int FileNum = FileNumFirst;
	//do
	//{
	//	CString csFinderString;
	//	csFinderString.Format(csFinderStringFormat, DirNum);
	//	CFileFind finder;
	//	BOOL bWorking = finder.FindFile(csFinderString.GetString());
	//	while (bWorking)
	//	{
	//		bWorking = finder.FindNextFile();
	//		SplitImagePath(finder.GetFilePath(), DestParentDir, DirNum, DestChildSuffix, DestFilePrefix, DestFileNumDigits, FileNum, DestFileExt);
	//		if ((FileNum >= FileNumFirst) && (FileNum <= FileNumLast))
	//		{
	//			SourceImageList.push_back(finder.GetFilePath());
	//			CombinedSourceImageSize += finder.GetLength();
	//		}
	//	}
	//	finder.Close();
	//	DirNum++;
	//} while (DirNum <= DirNumLast);

	CString csFinderString(_T("G:/RoavDashCam/20180520/MOVIE/2018_0520*.MP4"));
	CFileFind finder;
	BOOL bWorking = finder.FindFile(csFinderString.GetString());
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		SourceImageList.push_back(finder.GetFilePath());
		CombinedSourceImageSize += finder.GetLength();
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
				std::wcout << "\t" << argv[0] << " VideoName [-preset [ultrafast|superfast|veryfast|faster|fast|medium|slow|slower|veryslow]] [-crf [0-28]] PathToFirstFile.mp4 PathToLastFile.mp4" << std::endl;
			}
			else
			{
				std::vector<CString> SourceImageList;
				unsigned long long CombinedSourceImageSize = 0;
				CString csFFMPEGPath(FindEXEFromPath(_T("ffmpeg.exe")));
				CString csVideoName(argv[1]);
				CString csH265preset(_T("veryfast"));	// h.265 option: Valid presets are ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo. The default is medium.
				CString csH265crf(_T("23"));			// use instead of default - crf 28 (Added 4 / 05 / 2018)
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

				TCHAR szPath[MAX_PATH] = _T("");
				SHGetFolderPath(NULL, CSIDL_MYVIDEO, NULL, 0, szPath);
				PathAddBackslash(szPath);
				CString csImageDirectory(szPath);
				csImageDirectory.Append(csVideoName);
				CString csVideoFullPath(csImageDirectory); csVideoFullPath.Append(_T(".mp4"));
				if (csFFMPEGPath.GetLength() > 0)
				{
					vector<wchar_t *> args;
					vector<wstring> mycommand;

					TCHAR szFontFilePath[MAX_PATH] = _T("");
					SHGetFolderPath(NULL, CSIDL_FONTS, NULL, 0, szFontFilePath);
					PathAddBackslash(szFontFilePath);
					//PathAppend(szFontFilePath, _T("OCRAEXT"));
					PathAppend(szFontFilePath, _T("consola"));
					PathAddExtension(szFontFilePath, _T(".ttf"));
					CString csFontFilePath(szFontFilePath);
					csFontFilePath.Replace(_T("\\"), _T("/"));
					csFontFilePath.Replace(_T(":"), _T("\\\\:"));
					csVideoFullPath = csImageDirectory + _T(".mp4");
					args.clear();
					mycommand.clear();
					mycommand.push_back(csFFMPEGPath.GetString());
#ifdef _DEBUG
					mycommand.push_back(_T("-report"));
#endif
					mycommand.push_back(_T("-hide_banner"));

					CString csFilterCommand;
					int OutFileIndex = 0;
					for (auto SourceFile = SourceImageList.begin(); SourceFile != SourceImageList.end(); SourceFile++)
					{
						mycommand.push_back(_T("-i")); mycommand.push_back(QuoteFileName(SourceFile->GetString()).GetString());
						csFilterCommand.AppendFormat(_T("[%d:v]"), OutFileIndex++);
					}
					csFilterCommand.AppendFormat(_T("concat=n=%d:v=1[v];"), OutFileIndex);
					csFilterCommand.Append(_T("[v]"));
					csFilterCommand.Append(_T("setpts=(1/60)*PTS"));
					csFilterCommand.Append(_T(",drawtext=fontfile="));
					csFilterCommand.Append(csFontFilePath);
					csFilterCommand.Append(_T(":fontcolor=white:fontsize=80:y=main_h-text_h-50:x=50:text=WimsWorld"));
					csFilterCommand.Append(_T("[o]"));
					mycommand.push_back(_T("-filter_complex")); mycommand.push_back(csFilterCommand.GetString());
					mycommand.push_back(_T("-map")); mycommand.push_back(_T("[o]"));
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
						m_LogFile << "[" << getTimeISO8601() << "]";
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
						auto FPS = double(SourceImageList.size()) / double(TotalSeconds);
						std::wcout << "[" << getTimeISO8601() << "] encoded " << SourceImageList.size() << " frames in " << TotalSeconds << "s (" << FPS << "fps)" << std::endl;
						m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
						if (m_LogFile.is_open())
						{
							m_LogFile << "[" << getTimeISO8601() << "] encoded " << SourceImageList.size() << " frames in " << TotalSeconds << "s (" << FPS << "fps)" << std::endl;
							m_LogFile.close();
						}
					}

					try
					{
						CFileStatus StatusPicture, StatusVideo;
						if (TRUE == CFile::GetStatus(SourceImageList.begin()->GetString(), StatusPicture))
							if (TRUE == CFile::GetStatus(csVideoFullPath, StatusVideo))
							{
								m_LogFile.open(GetLogFileName().GetString(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
								if (m_LogFile.is_open())
								{
									m_LogFile << "[" << getTimeISO8601() << "] File Size: " << StatusVideo.m_size << std::endl;
									m_LogFile.close();
								}
								StatusVideo.m_ctime = StatusVideo.m_mtime = StatusPicture.m_ctime;
								CFile::SetStatus(csVideoFullPath, StatusVideo);
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
