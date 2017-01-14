
all:
	cd src;make;make nameserver;mv proxy ../;mv nameserver ../

clean:
	rm proxy;rm nameserver;cd src;make clean

handin:
	make clean; cd ..; tar cvf 1400012849&1400012837_handin.tar handin
