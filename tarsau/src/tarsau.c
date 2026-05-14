#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#define MAX_DOSYA 32
#define MAX_BOYUT (200 * 1024 * 1024L)

struct dosya_bilgi {
	char isim[256];
	mode_t izin;
	long boyut;
};

int metin_mi(const char *yol) {
	FILE *fp = fopen(yol, "rb");
	if (!fp)
		return 0;

	int c;
	while ((c = fgetc(fp)) != EOF) {
		if (c > 127)  {
			fclose(fp);
			return 0;
		}
		if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return 1;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Kullanim:\n");
		printf("  tarsau -b dosya1 dosya2 ... [-o arsiv.sau]\n");
		printf("  tarsau -a arsiv.sau [hedef_dizin]\n");
		return 1;
	}

	if (strcmp(argv[1], "-b") == 0) {
		char *dosyalar[MAX_DOSYA];
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
				if (dosya_sayisi >= MAX_DOSYA) {
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

		struct dosya_bilgi bilgiler[MAX_DOSYA];
		long toplam_boyut = 0;

		for (int i = 0; i < dosya_sayisi; i++) {
			struct stat st;

			if (stat(dosyalar[i], &st) != 0) {
				printf("%s: Dosya bulunamadi\n", dosyalar[i]);
				return 1;
			}

			if (!metin_mi(dosyalar[i])) {
				printf("%s giris dosyasinin formati uyumsuzdur!\n", dosyalar[i]);
				return 1;
			}

			strncpy(bilgiler[i].isim, dosyalar[i], 255);
			bilgiler[i].isim[255] = '\0';
			bilgiler[i].izin = st.st_mode;
			bilgiler[i].boyut = st.st_size;

			toplam_boyut += st.st_size;
		}

		if (toplam_boyut > MAX_BOYUT) {
			printf("Hata: Toplam dosya boyutu 200MB sinirini asiyor.\n");
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
