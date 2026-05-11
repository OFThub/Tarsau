# tarsau - Dosya Arsivleme Araci

## Aciklama

tar, rar, zip benzeri calisan ancak sikistirma yapmayan bir arsivleme programi. Metin dosyalarini tek bir `.sau` arsiv dosyasinda birlestirir ve geri acar.

## Derleme

```
make
```

## Kullanim

### Birlestirme

Birden fazla dosyayi tek bir arsiv dosyasinda birlestirir:

```
tarsau -b dosya1 dosya2 ... -o arsiv.sau
```

### Acma

Bir arsiv dosyasini acar ve icerigini cikarir:

```
tarsau -a arsiv.sau [hedef_dizin]
```

Hedef dizin belirtilmezse dosyalar mevcut dizine cikarilir.

## Arsiv Formati (.sau)

Arsiv dosyasi iki bolumden olusur:

1. **Organizasyon bolumu**: Ilk 10 byte, organizasyon bolumunun toplam boyutunu icerir. Ardindan pipe (`|`) karakteri ile ayrilmis kayitlar gelir. Her kayit su bilgileri icerir:
   - Dosya adi
   - Dosya izinleri
   - Dosya boyutu

2. **Icerik bolumu**: Tum dosyalarin icerikleri sirasiyla yer alir.

## Kisitlamalar

- Bir arsivde en fazla **32 dosya** bulunabilir.
- Toplam arsiv boyutu **200 MB**'i asamaz.
- Yalnizca **ASCII metin dosyalari** desteklenir.

## Gelistirme

Gelistirme asamalari commit gecmisinde takip edilebilir.
