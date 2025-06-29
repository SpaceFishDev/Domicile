#include <efi.h>
#include <efilib.h>
#include <elf.h>

#include <stdint.h>

typedef unsigned long long size_t;

typedef struct
{
	void *base_addr;
	size_t buffer_size;
	unsigned int width, height;
	unsigned int pixels_per_scanline;
} frame_buffer_t;

frame_buffer_t global_framebuffer;

frame_buffer_t *InitializeGOP()
{
	EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void **)&gop);
	if (EFI_ERROR(status))
	{
		Print(L"Unable to locate gop.\n\r");
		return NULL;
	}
	Print(L"GOP located\n\r");

	global_framebuffer.base_addr = (void *)(gop->Mode->FrameBufferBase);
	global_framebuffer.buffer_size = gop->Mode->FrameBufferSize;
	global_framebuffer.width = gop->Mode->Info->HorizontalResolution;
	global_framebuffer.height = gop->Mode->Info->VerticalResolution;
	global_framebuffer.pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

	return &global_framebuffer;
}

EFI_FILE *load_file(EFI_FILE *dir, CHAR16 *path, EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	EFI_FILE *loaded_file;

	EFI_LOADED_IMAGE_PROTOCOL *loaded_image;

	system_table->BootServices->HandleProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (void **)&loaded_image);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;

	system_table->BootServices->HandleProtocol(loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void **)&file_system);

	if (dir == NULL)
	{
		file_system->OpenVolume(file_system, &dir);
	}
	EFI_STATUS s = dir->Open(dir, &loaded_file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if (s != EFI_SUCCESS)
	{
		return NULL;
	}
	return loaded_file;
}

int memcmp(const void *a, const void *b, int n)
{
	for (int i = 0; i < n; ++i)
	{
		if (((const char *)a)[i] < ((const char *)b)[i])
		{
			return -1;
		}
		if (((const char *)a)[i] > ((const char *)b)[i])
		{
			return 1;
		}
	}
	return 0;
}

#define PSF1_MAGIC_0 0x36
#define PSF1_MAGIC_1 0x04

typedef struct
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} psf1_header_t;

typedef struct
{
	psf1_header_t *psf1_header;
	void *glyph_buffer;

} psf1_font_t;

psf1_font_t *load_psf1_font(EFI_FILE *dir, CHAR16 *path, EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	EFI_FILE *font = load_file(dir, path, image_handle, system_table);
	if (!font)
	{
		return NULL;
	}

	psf1_header_t *font_header;
	system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(psf1_header_t), (void **)&font_header);

	UINTN size = sizeof(psf1_header_t);
	font->Read(font, &size, font_header);

	if (font_header->magic[0] != PSF1_MAGIC_0 || font_header->magic[1] != PSF1_MAGIC_1)
	{
		return NULL;
	}

	UINTN glyph_buffer_sz = font_header->charsize * 256;
	if (font_header->mode == 1)
	{
		glyph_buffer_sz *= 2;
	}
	void *glyph_buffer;
	{
		font->SetPosition(font, sizeof(psf1_header_t));
		system_table->BootServices->AllocatePool(EfiLoaderData, glyph_buffer_sz, (void **)&glyph_buffer);
		font->Read(font, &glyph_buffer_sz, glyph_buffer);
	}
	psf1_font_t *finished_font;
	system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(psf1_font_t), (void **)&finished_font);
	finished_font->psf1_header = font_header;
	finished_font->glyph_buffer = glyph_buffer;
	return finished_font;
}

typedef struct
{
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN map_size;
	UINTN map_descriptor_size;
} mem_info_t;

typedef struct
{
	frame_buffer_t *frame_buffer;
	psf1_font_t *font;
	mem_info_t *memory_info;
} boot_info_t;

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table)
{
	InitializeLib(image_handle, system_table);
	Print(L"Hello, World!\n\r");
	EFI_FILE *kernel;
	kernel = load_file(NULL, L"kernel.elf", image_handle, system_table);
	if (!kernel)
	{
		Print(L"Did not load kernel.\n\r");
		return -1;
	}
	Print(L"Kernel loaded successfully.\n\r");

	Elf64_Ehdr header;
	{
		UINTN file_info_size;
		EFI_FILE_INFO *file_info;
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_info_size, NULL);
		system_table->BootServices->AllocatePool(EfiLoaderData, file_info_size, (void **)&file_info);
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_info_size, (void **)&file_info);

		UINTN size = sizeof(header);
		kernel->Read(kernel, &size, &header);
	}
	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC || header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT)
	{
		Print(L"Kernel format is bad.\n\r");
	}
	else
	{
		Print(L"Kernel header successfully verified.\n\r");
	}

	Elf64_Phdr *phdrs;
	{
		kernel->SetPosition(kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		system_table->BootServices->AllocatePool(EfiLoaderData, size, (void **)&phdrs);
		kernel->Read(kernel, &size, phdrs);
	}
	for (Elf64_Phdr *phdr = phdrs; (char *)phdr < (char *)phdrs + header.e_phnum * header.e_phentsize; phdr = (Elf64_Phdr *)((char *)phdr + header.e_phentsize))
	{
		switch (phdr->p_type)
		{
		case PT_LOAD:
		{
			int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			Elf64_Addr segment = phdr->p_paddr;
			system_table->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);
			kernel->SetPosition(kernel, phdr->p_offset);
			UINTN size = phdr->p_filesz;
			kernel->Read(kernel, &size, (void *)segment);
		}
		break;
		}
	}
	Print(L"Kernel Loaded\n\r");

	psf1_font_t *psf1_font = load_psf1_font(NULL, L"zap-vga16.psf", image_handle, system_table);
	if (!psf1_font)
	{
		Print(L"Could not load font.\n\r");
	}
	else
	{
		Print(L"Font found: Char size = %d\n\r", psf1_font->psf1_header->charsize);
	}

	frame_buffer_t *frame_buffer = InitializeGOP();

	Print(L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d\n\rHeight: %d\n\rPixels Per Scanline: %d\n\r", frame_buffer->base_addr, frame_buffer->buffer_size, frame_buffer->width, frame_buffer->height, frame_buffer->pixels_per_scanline);

	EFI_MEMORY_DESCRIPTOR *map = NULL;
	UINTN map_size, map_key;
	UINTN descriptor_size;
	UINT32 descriptor_version;
	{
		system_table->BootServices->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);
		system_table->BootServices->AllocatePool(EfiLoaderData, map_size, (void **)&map);
		system_table->BootServices->GetMemoryMap(&map_size, map, &map_key, &descriptor_size, &descriptor_version);
	}

	void (*kernel_start)(boot_info_t *) = ((__attribute__((sysv_abi)) void (*)(boot_info_t *))header.e_entry);

	boot_info_t boot_info;
	boot_info.font = psf1_font;
	boot_info.frame_buffer = frame_buffer;
	mem_info_t mem_info = (mem_info_t){map, map_size, descriptor_size};
	boot_info.memory_info = &mem_info;

	system_table->BootServices->ExitBootServices(image_handle, map_key);

	kernel_start(&boot_info);

	return 0;
}
