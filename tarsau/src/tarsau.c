#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Kullanim:\n");
		printf("  tarsau -b dosya1 dosya2 ... [-o arsiv.sau]\n");
		printf("  tarsau -a arsiv.sau [hedef_dizin]\n");
		return 1;
	}

	if (strcmp(argv[1], "-b") == 0) {
		char *dosyalar[32];
		int dosya_sayisi = 0;
		char *cikti_adi = "a.sau";

		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-o") == 0) {
				if (i + 1 < argc) {
					cikti_adi = argv[i + 1];
					i++;
				} else {
					printf("Hata: -o parametresinden sonra dosya adi belirtilmedi.\n");
					return 1;
				}
			} else {
				if (dosya_sayisi >= 32) {
					printf("Hata: En fazla 32 dosya arsivlenebilir.\n");
					return 1;
				}
				dosyalar[dosya_sayisi] = argv[i];
				dosya_sayisi++;
			}
		}

		if (dosya_sayisi == 0) {
			printf("Hata: En az bir giris dosyasi belirtmelisiniz.\n");
			return 1;
		}

		printf("Birlestirme modu: %d dosya -> %s\n", dosya_sayisi, cikti_adi);

	} else if (strcmp(argv[1], "-a") == 0) {
		if (argc < 3) {
			printf("Hata: Arsiv dosyasi belirtilmedi.\n");
			return 1;
		}

		char *arsiv_adi = argv[2];
		int len = strlen(arsiv_adi);

		if (len < 4 || strcmp(arsiv_adi + len - 4, ".sau") != 0) {
			printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
			return 1;
		}

		if (argc > 4) {
			printf("Hata: Fazla parametre belirtildi.\n");
			return 1;
		}

		char *hedef_dizin = NULL;
		if (argc == 4)
			hedef_dizin = argv[3];

		printf("Acma modu: %s -> %s\n", arsiv_adi,
			hedef_dizin ? hedef_dizin : "(gecerli dizin)");

	} else {
		printf("Hata: Gecersiz parametre '%s'\n", argv[1]);
		return 1;
	}

	return 0;
}
