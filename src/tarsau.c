#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MAX_DOSYA 32
#define MAX_BOYUT (200 * 1024 * 1024L)
#define BUF_SIZE 4096

struct dosya_bilgi {
	char isim[256];
	char yol[256];
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

int arsiv_olustur(struct dosya_bilgi *bilgiler, int dosya_sayisi, const char *cikti_adi) {
	char org[MAX_DOSYA * 300] = "";

	for (int i = 0; i < dosya_sayisi; i++) {
		char kayit[512];
		sprintf(kayit, "|%s,%o,%ld|", bilgiler[i].isim,
			bilgiler[i].izin & 0777, bilgiler[i].boyut);
		strcat(org, kayit);
	}

	long org_uzunluk = strlen(org);
	char baslik[11];
	sprintf(baslik, "%010ld", org_uzunluk);

	FILE *cikti = fopen(cikti_adi, "wb");
	if (!cikti) {
		printf("Hata: %s dosyasi olusturulamadi.\n", cikti_adi);
		return 0;
	}

	if (fwrite(baslik, 1, 10, cikti) != 10 ||
		fwrite(org, 1, org_uzunluk, cikti) != (size_t)org_uzunluk) {
		perror("Hata: Arsiv dosyasina yazma basarisiz");
		fclose(cikti);
		return 0;
	}

	char buf[BUF_SIZE];
	for (int i = 0; i < dosya_sayisi; i++) {
		if (bilgiler[i].boyut == 0)
			continue;

		FILE *fp = fopen(bilgiler[i].yol, "rb");
		if (!fp) {
			perror(bilgiler[i].yol);
			fclose(cikti);
			return 0;
		}

		size_t n;
		while ((n = fread(buf, 1, BUF_SIZE, fp)) > 0) {
			if (fwrite(buf, 1, n, cikti) != n) {
				perror("Hata: Arsiv dosyasina yazma basarisiz");
				fclose(fp);
				fclose(cikti);
				return 0;
			}
		}

		fclose(fp);
	}

	fclose(cikti);
	return 1;
}

int kayitlari_parse_et(const char *org, struct dosya_bilgi *bilgiler, int *dosya_sayisi) {
	char *kopya = strdup(org);
	if (!kopya)
		return 0;

	*dosya_sayisi = 0;
	char *p = kopya;

	while (*p) {
		if (*p != '|') {
			p++;
			continue;
		}
		p++;

		char *son = strchr(p, '|');
		if (!son)
			break;
		*son = '\0';

		if (strlen(p) == 0) {
			p = son + 1;
			continue;
		}

		if (*dosya_sayisi >= MAX_DOSYA) {
			free(kopya);
			return 0;
		}

		char *isim = strtok(p, ",");
		char *izin_str = strtok(NULL, ",");
		char *boyut_str = strtok(NULL, ",");

		if (!isim || !izin_str || !boyut_str) {
			free(kopya);
			return 0;
		}

		long boyut = atol(boyut_str);
		if (boyut < 0) {
			free(kopya);
			return 0;
		}

		strncpy(bilgiler[*dosya_sayisi].isim, isim, 255);
		bilgiler[*dosya_sayisi].isim[255] = '\0';
		bilgiler[*dosya_sayisi].izin = (mode_t)strtol(izin_str, NULL, 8);
		bilgiler[*dosya_sayisi].boyut = boyut;

		(*dosya_sayisi)++;
		p = son + 1;
	}

	free(kopya);
	return (*dosya_sayisi > 0);
}

int arsiv_ac(const char *arsiv_adi, const char *hedef_dizin) {
	FILE *fp = fopen(arsiv_adi, "rb");
	if (!fp) {
		printf("Arsiv dosyasi acilmadi!\n");
		return 0;
	}

	char baslik[11];
	if (fread(baslik, 1, 10, fp) != 10) {
		printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
		fclose(fp);
		return 0;
	}
	baslik[10] = '\0';

	for (int i = 0; i < 10; i++) {
		if (baslik[i] < '0' || baslik[i] > '9') {
			printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
			fclose(fp);
			return 0;
		}
	}

	long org_uzunluk = atol(baslik);

	if (org_uzunluk <= 0) {
		printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
		fclose(fp);
		return 0;
	}

	struct stat arsiv_st;
	if (fstat(fileno(fp), &arsiv_st) != 0 || arsiv_st.st_size < 10 + org_uzunluk) {
		printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
		fclose(fp);
		return 0;
	}

	char *org = malloc(org_uzunluk + 1);
	if (!org) {
		perror("Hata: Bellek ayrilamadi");
		fclose(fp);
		return 0;
	}

	if ((long)fread(org, 1, org_uzunluk, fp) != org_uzunluk) {
		printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
		free(org);
		fclose(fp);
		return 0;
	}
	org[org_uzunluk] = '\0';

	struct dosya_bilgi bilgiler[MAX_DOSYA];
	int dosya_sayisi;

	if (!kayitlari_parse_et(org, bilgiler, &dosya_sayisi)) {
		printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
		free(org);
		fclose(fp);
		return 0;
	}

	free(org);

	if (hedef_dizin) {
		struct stat st;
		if (stat(hedef_dizin, &st) != 0) {
			if (mkdir(hedef_dizin, 0755) != 0) {
				perror(hedef_dizin);
				fclose(fp);
				return 0;
			}
		}
		if (chdir(hedef_dizin) != 0) {
			printf("Hata: %s dizinine gecilemedi.\n", hedef_dizin);
			fclose(fp);
			return 0;
		}
	}

	char buf[BUF_SIZE];
	for (int i = 0; i < dosya_sayisi; i++) {
		FILE *out = fopen(bilgiler[i].isim, "wb");
		if (!out) {
			printf("Hata: %s dosyasi olusturulamadi.\n", bilgiler[i].isim);
			fclose(fp);
			return 0;
		}

		long kalan = bilgiler[i].boyut;
		while (kalan > 0) {
			size_t oku = (kalan > BUF_SIZE) ? BUF_SIZE : kalan;
			size_t n = fread(buf, 1, oku, fp);
			if (n == 0) {
				printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
				fclose(out);
				fclose(fp);
				return 0;
			}
			if (fwrite(buf, 1, n, out) != n) {
				perror("Hata: Dosya yazma basarisiz");
				fclose(out);
				fclose(fp);
				return 0;
			}
			kalan -= n;
		}

		fclose(out);
		chmod(bilgiler[i].isim, bilgiler[i].izin);
	}

	fclose(fp);

	if (hedef_dizin)
		printf("%s dizininde ", hedef_dizin);

	for (int i = 0; i < dosya_sayisi; i++) {
		if (i > 0)
			printf(", ");
		printf("%s", bilgiler[i].isim);
	}
	printf(" dosyalari acildi.\n");

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

		for (int i = 0; i < dosya_sayisi; i++) {
			if (strlen(dosyalar[i]) > 255) {
				printf("Hata: Dosya adi cok uzun: %s\n", dosyalar[i]);
				return 1;
			}
			for (int j = i + 1; j < dosya_sayisi; j++) {
				if (strcmp(dosyalar[i], dosyalar[j]) == 0) {
					printf("Hata: Ayni dosya birden fazla kez belirtilmis: %s\n", dosyalar[i]);
					return 1;
				}
			}
		}

		struct dosya_bilgi bilgiler[MAX_DOSYA];
		long toplam_boyut = 0;

		for (int i = 0; i < dosya_sayisi; i++) {
			struct stat st;

			if (stat(dosyalar[i], &st) != 0) {
				printf("%s: Dosya bulunamadi\n", dosyalar[i]);
				return 1;
			}

			if (st.st_size > 0 && !metin_mi(dosyalar[i])) {
				printf("%s giris dosyasinin formati uyumsuzdur!\n", dosyalar[i]);
				return 1;
			}

			const char *temel_isim = strrchr(dosyalar[i], '/');
			temel_isim = temel_isim ? temel_isim + 1 : dosyalar[i];

			strncpy(bilgiler[i].isim, temel_isim, 255);
			bilgiler[i].isim[255] = '\0';
			strncpy(bilgiler[i].yol, dosyalar[i], 255);
			bilgiler[i].yol[255] = '\0';
			bilgiler[i].izin = st.st_mode;
			bilgiler[i].boyut = st.st_size;

			toplam_boyut += st.st_size;
		}

		if (toplam_boyut > MAX_BOYUT) {
			printf("Hata: Toplam dosya boyutu 200MB sinirini asiyor.\n");
			return 1;
		}

		if (!arsiv_olustur(bilgiler, dosya_sayisi, cikti_adi))
			return 1;

		printf("Dosyalar basariyla birlestirildi.\n");

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

		if (!arsiv_ac(arsiv_adi, hedef_dizin))
			return 1;

	} else {
		printf("Hata: Gecersiz parametre '%s'\n", argv[1]);
		return 1;
	}

	return 0;
}
