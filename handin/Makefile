
all:
	cd src;make;make nameserver;mv proxy ../;mv nameserver ../

clean:
	rm proxy;rm nameserver;cd src;make clean

clobber: clean
	cd src; make clobber

handin:
	make clean; cd ..; tar cvf handin.tar handin
