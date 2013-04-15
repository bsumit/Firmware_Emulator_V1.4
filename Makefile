Iridium_Emulator : Iridium_Emulator_Basic.o Emulator_Properties.o Config_File_read.o send_data.o
	gcc -o Iridium_Emulator Iridium_Emulator_Basic.o Emulator_Properties.o Config_File_read.o send_data.o

Iridium_Emulator_Basic.o : Iridium_Emulator_Basic.c Emulator_Properties.h Config_File_read.h send_data.h
	gcc -c Iridium_Emulator_Basic.c
Emulator_Properties.o : Emulator_Properties.c Iridium_Emulator_Basic.h Config_File_read.h
	gcc -c Emulator_Properties.c
ConfigFile_read.o : Config_File_read.c Config_File_read.h Iridium_Emulator_Basic.h
	gcc -c ConfigFile.c
send_data.o : send_data.c send_data.h Config_File_read.h Iridium_Emulator_Basic.h
	gcc -c send_data.c
clean : 
	rm Iridium_Emulator Iridium_Emulator Iridium_Emulator_Basic.o Emulator_Properties.o Config_File_read.o send_data.o



