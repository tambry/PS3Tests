#include <stdio.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/l10n.h>

inline static const char* CodeToName(int code)
{
	switch (code)
	{
	case L10N_UTF8: return "L10N_UTF8";
	case L10N_UTF16: return "L10N_UTF16";
	case L10N_UTF32: return "L10N_UTF32";
	case L10N_UCS2: return "L10N_UCS2";
	case L10N_UCS4: return "L10N_UCS4";
	case L10N_ISO_8859_1: return "L10N_ISO_8859_1";
	case L10N_ISO_8859_2: return "L10N_ISO_8859_2";
	case L10N_ISO_8859_3: return "L10N_ISO_8859_3";
	case L10N_ISO_8859_4: return "L10N_ISO_8859_4";
	case L10N_ISO_8859_5: return "L10N_ISO_8859_5";
	case L10N_ISO_8859_6: return "L10N_ISO_8859_6";
	case L10N_ISO_8859_7: return "L10N_ISO_8859_7";
	case L10N_ISO_8859_8: return "L10N_ISO_8859_8";
	case L10N_ISO_8859_9: return "L10N_ISO_8859_9";
	case L10N_ISO_8859_10: return "L10N_ISO_8859_10";
	case L10N_ISO_8859_11: return "L10N_ISO_8859_11";
	case L10N_ISO_8859_13: return "L10N_ISO_8859_13";
	case L10N_ISO_8859_14: return "L10N_ISO_8859_14";
	case L10N_ISO_8859_15: return "L10N_ISO_8859_15";
	case L10N_ISO_8859_16: return "L10N_ISO_8859_16";
	case L10N_CODEPAGE_437: return "L10N_CODEPAGE_437";
	case L10N_CODEPAGE_850: return "L10N_CODEPAGE_850";
	case L10N_CODEPAGE_863: return "L10N_CODEPAGE_863";
	case L10N_CODEPAGE_866: return "L10N_CODEPAGE_866";
	case L10N_CODEPAGE_932: return "L10N_CODEPAGE_932";
	case L10N_CODEPAGE_936: return "L10N_CODEPAGE_936";
	case L10N_CODEPAGE_949: return "L10N_CODEPAGE_949";
	case L10N_CODEPAGE_950: return "L10N_CODEPAGE_950";
	case L10N_CODEPAGE_1251: return "L10N_CODEPAGE_1251";
	case L10N_CODEPAGE_1252: return "L10N_CODEPAGE_1252";
	case L10N_EUC_CN: return "L10N_EUC_CN";
	case L10N_EUC_JP: return "L10N_EUC_JP";
	case L10N_EUC_KR: return "L10N_EUC_KR";
	case L10N_ISO_2022_JP: return "L10N_ISO_2022_JP";
	case L10N_ARIB: return "L10N_ARIB";
	case L10N_HZ: return "L10N_HZ";
	case L10N_GB18030: return "L10N_GB18030";
	case L10N_RIS_506: return "L10N_RIS_506";
	case L10N_CODEPAGE_852: return "L10N_CODEPAGE_852";
	case L10N_CODEPAGE_1250: return "L10N_CODEPAGE_1250";
	case L10N_CODEPAGE_737: return "L10N_CODEPAGE_737";
	case L10N_CODEPAGE_1253: return "L10N_CODEPAGE_1253";
	case L10N_CODEPAGE_857: return "L10N_CODEPAGE_857";
	case L10N_CODEPAGE_1254: return "L10N_CODEPAGE_1254";
	case L10N_CODEPAGE_775: return "L10N_CODEPAGE_775";
	case L10N_CODEPAGE_1257: return "L10N_CODEPAGE_1257";
	case L10N_CODEPAGE_855: return "L10N_CODEPAGE_855";
	case L10N_CODEPAGE_858: return "L10N_CODEPAGE_858";
	case L10N_CODEPAGE_860: return "L10N_CODEPAGE_860";
	case L10N_CODEPAGE_861: return "L10N_CODEPAGE_861";
	case L10N_CODEPAGE_865: return "L10N_CODEPAGE_865";
	case L10N_CODEPAGE_869: return "L10N_CODEPAGE_869";
	case _L10N_CODE_: return "_L10N_CODE_";
	}
}

int main(void)
{
	printf("TEST00004 by tambre.\n");
	printf("Generating all the possible converter IDs.\n");
	printf("Firmware 3.10+ required.\n\n");

	printf("Loading CELL_SYSMODULE_L10N...\n");
	int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_L10N);

	if (ret != CELL_OK)
	{
		printf("cellSysmoduleLoadModule(CELL_SYSMODULE_L10N) error code: 0x%x\n", ret);
		sys_process_exit(1);
	}

	L10nCode codes[] = { L10N_UTF8, L10N_UTF16, L10N_UTF32, L10N_UCS2, L10N_UCS4, L10N_ISO_8859_1, L10N_ISO_8859_2, L10N_ISO_8859_3, L10N_ISO_8859_4, L10N_ISO_8859_5, L10N_ISO_8859_6, L10N_ISO_8859_7,
						 L10N_ISO_8859_8, L10N_ISO_8859_9, L10N_ISO_8859_10, L10N_ISO_8859_11, L10N_ISO_8859_13, L10N_ISO_8859_14, L10N_ISO_8859_15, L10N_ISO_8859_16, L10N_CODEPAGE_437, L10N_CODEPAGE_850,
						 L10N_CODEPAGE_863, L10N_CODEPAGE_866, L10N_CODEPAGE_932, L10N_CODEPAGE_936, L10N_CODEPAGE_949, L10N_CODEPAGE_950, L10N_CODEPAGE_1251, L10N_CODEPAGE_1252, L10N_EUC_CN, L10N_EUC_JP,
						 L10N_EUC_KR, L10N_ISO_2022_JP, L10N_ARIB, L10N_HZ, L10N_GB18030, L10N_RIS_506, L10N_CODEPAGE_852, L10N_CODEPAGE_1250, L10N_CODEPAGE_737, L10N_CODEPAGE_1253, L10N_CODEPAGE_857,
						 L10N_CODEPAGE_1254, L10N_CODEPAGE_775, L10N_CODEPAGE_1257, L10N_CODEPAGE_855, L10N_CODEPAGE_858, L10N_CODEPAGE_860, L10N_CODEPAGE_861, L10N_CODEPAGE_865, L10N_CODEPAGE_869, _L10N_CODE_ };

	int codesLength = sizeof(codes) / sizeof(*codes);
	printf("Amount of IDs: %d\n\n", codesLength);
	printf("Generating:\n");

	for (int c = 0; c < codesLength; c++)
	{
		for (int i = 0; i < codesLength; i++)
		{
			ret = l10n_get_converter(codes[c], codes[i]);
			if (ret < 0)
			{
				printf("%s to %s errored!\n", CodeToName(codes[c]), CodeToName(codes[i]));
				printf("Error code: %d\n", ret);
			}
			else
			{
				printf("%s to %s: %d\n", CodeToName(codes[c]), CodeToName(codes[i]), ret);
			}
		}
	}

	return 0;
}