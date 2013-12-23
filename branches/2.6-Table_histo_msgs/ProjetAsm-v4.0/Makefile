CC=asx8051
LINK=aslink
CFLAGS=-jlosxg
EXEC=init.ihx
OBJETS_REL=init.rel rs232.rel crc16.rel interruption.rel cligno_run.rel can.rel fct_ident.rel fct_entre_tor_AP8.rel fct_sortie_tor_AP8.rel fct_entre_ana.rel

all: $(EXEC)
	
%.ihx: $(OBJETS_REL)
	$(LINK) -nf init
	
%.rel: %.asm cst.asm conf.asm
	$(CC) $(CFLAGS) $<
