# NUC240-SPI-FLASH-TOOL
NUC240 SPI FLASH TOOL
FW:
void USB_Process(void)處理USB丟過來COMMAND
SPI_FLASH.C處理FLASH的ERASE, PROGRAM, READ,
USB底層都相同，直接可以把CODE移到NUC123上面。

AP.
unsigned int FW_version(void) =>讀FW 版本
unsigned int TARGET_DEVICE_ID(void) =>讀取目標SPI FLASH的ID
unsigned int ERASE_CHIP(void) =>SPI FLASH CHIP FLASH ERASE
unsigned int PROGRAM_FLASH(unsigned int address, unsigned char * u8Data) =>寫入值到FLASH，含VERIFY.
ADDRESS，為FLASH寫入的地址。 USB封包一次訂48筆，傳進去的ARRAY 只能48，

unsigned int Read_FLASH(unsigned int address, unsigned char * u8Data)=>讀取SPI FLASH的資料
ADDRESS，為FLASH寫入的地址。 USB封包一次訂48筆，傳進去的ARRAY 只能48，
