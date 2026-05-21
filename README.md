# tarsau - Dosya Arsivleme Araci

## Aciklama

tar, rar, zip benzeri calisan ancak sikistirma yapmayan bir arsivleme programi. Metin dosyalarini tek bir `.sau` arsiv dosyasinda birlestirir ve geri acar.

## Derleme

```
make
```

Temizlemek icin:

```
make clean
```

## Kullanim

### Birlestirme

```
./tarsau -b dosya1.txt dosya2.txt -o arsiv.sau
```

`-o` belirtilmezse cikti dosyasi `a.sau` olur:

```
./tarsau -b dosya1.txt dosya2.txt
```

### Acma

Gecerli dizine acma:

```
./tarsau -a arsiv.sau
```

Belirli bir dizine acma:

```
./tarsau -a arsiv.sau hedef_dizin
```

## Arsiv Formati (.sau)

Arsiv dosyasi iki bolumden olusur:

1. **Organizasyon bolumu**: Ilk 10 byte, organizasyon bolumunun toplam boyutunu icerir (sifir ile doldurulmus, ornegin `0000000035`). Ardindan pipe (`|`) karakteri ile ayrilmis kayitlar gelir: `|dosya_adi,izinler,boyut|`

2. **Icerik bolumu**: Tum dosyalarin icerikleri sirasiyla yer alir.

Ornek: iki dosya (t1: 100 byte, izin 644 ve t2: 50 byte, izin 755) icin:

```
0000000035|t1,644,100||t2,755,50|<t1 icerigi><t2 icerigi>
```

## Kisitlamalar

- En fazla **32 dosya** arsivlenebilir.
- Toplam boyut **200 MB**'i asamaz.
- Yalnizca **ASCII metin dosyalari** desteklenir.

## Gelistirme

Gelistirme asamalari commit gecmisinde takip edilebilir.
