#include <stdio.h>
#include <stdlib.h>

#define FST_NFO	0x00000034

typedef unsigned long	u32;
typedef unsigned short	u16;
typedef unsigned char	u8;

u32 be32(u32 value)
{
	u32 out = 0;
	out |= ( (value & 0x000000FFUL) << 24);
	out |= ( (value & 0x0000FF00UL) <<  8);
	out |= ( (value & 0x00FF0000UL) >>  8);
	out |= ( (value & 0xFF000000UL) >> 24);
	return out;
}

typedef struct fst_t
{
	u32 strOffset;
	u32 offset;
	u32 size;
} fst_t;
void swapFstEntry(fst_t* entry)
{
	entry->strOffset = be32(entry->strOffset);
	entry->offset = be32(entry->offset);
	entry->size = be32(entry->size);
}
fst_t readFstEntry(char* data)
{
	fst_t entry;
	entry.strOffset = *(u32*)(data + 0x00);
	entry.offset = *(u32*)(data + 0x04);
	entry.size = *(u32*)(data + 0x08);
	swapFstEntry(&entry);
	return entry;
}
void showFstEntry(fst_t* entry)
{
	printf("strOffset: %#lx\n", entry->strOffset);
	printf("offset: %#lx\n", entry->offset);
	printf("size: %#lx\n", entry->size);
}
int isFile(fst_t* entry)
{
	return !(entry->strOffset & 0xFF000000UL);
}
int isDir(fst_t* entry)
{
	return (entry->strOffset & 0xFF000000UL);
}
void showFstEntryName(fst_t* entry, char * string_table)
{
	printf("name: %s\n", string_table + (entry->strOffset & 0x00FFFFFF));
}
const char * getFstEntryName(fst_t* entry, char * string_table)
{
	return string_table + (entry->strOffset & 0x00FFFFFF);
}

int ShowUsage(const char* progname)
{
	printf("Usage: %s <memdump>\n", progname);
	return EXIT_FAILURE;
}
int main(int argc, char ** argv)
{
	printf("FST Viewer by megazig 2013\n");
	if (argc != 2)
	{
		return ShowUsage(argv[0]);
	}

	FILE * fp = fopen(argv[1], "rb");
	if (!fp)
	{
		printf("Couldn't open memdump %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char * data = malloc(length);
	if (!data)
	{
		printf("Couldn't allocate memory for reading\n");
		fclose(fp);
		return EXIT_FAILURE;
	}

	int read = fread(data, 1, length, fp);
	if (read != length)
	{
		printf("Read %d expected %d\n", read, length);
		free(data);
		fclose(fp);
		return EXIT_FAILURE;
	}

	fclose(fp);

	u32 fst_offset = *(u32*)(data + FST_NFO);
	fst_offset = be32(fst_offset);
	fst_offset &= 0x7FFFFFFF;

	fst_t entry = readFstEntry(data + fst_offset);
	u32 file_count = entry.size;
	char * string_table = data + fst_offset + (file_count+0)*sizeof(fst_t);

	char fpout_name[0x200] = {0};
	snprintf(fpout_name, 0x200, "%s.fst", argv[1]);
	FILE * fpout = fopen(fpout_name, "wb");
	if (!fpout)
	{
		printf("Couldn't open %s for writing\n", argv[2]);
		free(data);
		return EXIT_FAILURE;
	}

	fprintf(fpout, "RVL-%c%c%c%c%c%c\n\n", data[0], data[1], data[2], data[3], data[4], data[5]);
	fprintf(fpout, "root\n");
	fprintf(fpout, "%08lX: %08lX %08lX %08lX\n\n", fst_offset | 0x80000000, entry.strOffset, entry.offset, entry.size);

	u32 ii = 0;
	for (ii = 1; ii < file_count; ii++)
	{
		fst_t ent = readFstEntry(data + fst_offset + (ii+0)*sizeof(fst_t));
		const char * name = getFstEntryName(&ent, string_table);
		fprintf(fpout, "%s\n", name);
		fprintf(fpout, "%08lX: %08lX %08lX %08lX\n\n", fst_offset + ii*sizeof(fst_t) | 0x80000000, ent.strOffset, ent.offset, ent.size);
	}

	free(data);
	fclose(fpout);

	printf("Completed. Output @ %s\n", fpout_name);
	return EXIT_SUCCESS;
}
