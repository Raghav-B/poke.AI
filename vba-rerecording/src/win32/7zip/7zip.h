#ifndef _7ZIP_DEC_HEADER
#define _7ZIP_DEC_HEADER

// 7zip file extraction
// NOTE: if you want to add support for opening files within archives to something,
// consider using the higher-level interface provided by OpenArchive.h instead

void InitDecoder();
void CleanupDecoder();
const char* GetSupportedFormatsFilter();

// simplest way of extracting a file after calling InitDecoder():
// int size = ArchiveFile(filename).ExtractItem(0, buf, sizeof(buf));

struct ArchiveFile
{
	ArchiveFile(const char* filename, const char* displayFilename=0);
	virtual ~ArchiveFile();

	int GetNumItems();
	int GetItemSize(int item);
	const char* GetItemName(int item);
	int ExtractItem(int item, unsigned char* outBuffer, int bufSize) const; // returns size, or 0 if failed
	int ExtractItem(int item, const char* outFilename) const;

	bool IsCompressed();
	const char* GetArchiveTypeName();
	const char* GetArchiveFileName() { return m_displayFilename ? m_displayFilename : m_filename; }

	bool m_userMadeSelection;

protected:
	struct ArchiveItem
	{
		int size;
		char* name;
	};
	ArchiveItem* m_items;
	int m_numItems;
	int m_typeIndex;
	char* m_filename;
	char* m_displayFilename;
};

#endif
