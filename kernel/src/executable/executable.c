#include "executable.h"
#include "../utils/string.h"
#include "../memory/malloc.h"
#include "../scheduler/scheduler.h"
#include "../filesystem/filesystem.h"
#include "../kernel-trace/kernel_trace.h"
#include <elf.h>
#include <stdint.h>
#include <stddef.h>

void *load_elf_reloc(uint8_t *elf_buffer, void *load_base)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_buffer;

    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3)
    {
        return NULL;
    }

    Elf64_Phdr *phdr = (Elf64_Phdr *)(elf_buffer + ehdr->e_phoff);

    uintptr_t min_vaddr = UINTPTR_MAX;
    uintptr_t max_vaddr = 0;
    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        if (phdr[i].p_type != PT_LOAD)
            continue;
        if (phdr[i].p_memsz == 0)
            continue;
        if (phdr[i].p_vaddr < min_vaddr)
            min_vaddr = phdr[i].p_vaddr;
        uintptr_t seg_end = phdr[i].p_vaddr + phdr[i].p_memsz;
        if (seg_end > max_vaddr)
            max_vaddr = seg_end;
    }

    if (min_vaddr == UINTPTR_MAX)
        return NULL;

    uintptr_t slide = (uintptr_t)load_base - min_vaddr;

    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        if (phdr[i].p_type != PT_LOAD)
            continue;

        uint8_t *src = elf_buffer + phdr[i].p_offset;
        void *dest = (void *)(slide + phdr[i].p_vaddr);

        if (phdr[i].p_filesz > 0)
            memcpy(dest, src, phdr[i].p_filesz);

        if (phdr[i].p_memsz > phdr[i].p_filesz)
        {
            uint8_t *bss_start = (uint8_t *)dest + phdr[i].p_filesz;
            size_t bss_len = phdr[i].p_memsz - phdr[i].p_filesz;
            memset(bss_start, 0, bss_len);
        }
    }

    if (ehdr->e_type == ET_EXEC)
    {
    }

    return (void *)(slide + ehdr->e_entry);
}

void start_proc_from_path(char *path)
{
    KERN_TRACE_FUNC;
    uint64_t fd = fs_open(path, false);
    if (fd == (uint64_t)-1)
        return;
    uint64_t fz = fs_get_file_size(fd);

    uint8_t *file_buf = malloc(fz);
    if (!file_buf)
    {
        fs_close(fd);
        return;
    }
    fs_read(file_buf, fd);
    fs_close(fd);

    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_buf;
    Elf64_Phdr *phdr = (Elf64_Phdr *)(file_buf + ehdr->e_phoff);

    uintptr_t min_vaddr = UINTPTR_MAX;
    uintptr_t max_vaddr = 0;
    for (int i = 0; i < ehdr->e_phnum; ++i)
    {
        if (phdr[i].p_type != PT_LOAD)
            continue;
        if (phdr[i].p_memsz == 0)
            continue;
        if (phdr[i].p_vaddr < min_vaddr)
            min_vaddr = phdr[i].p_vaddr;
        uintptr_t seg_end = phdr[i].p_vaddr + phdr[i].p_memsz;
        if (seg_end > max_vaddr)
            max_vaddr = seg_end;
    }
    if (min_vaddr == UINTPTR_MAX)
    {
        free(file_buf);
        return;
    }

    size_t prog_size = max_vaddr - min_vaddr;
    size_t num_pages = (prog_size + 0xfff) / 0x1000;

    uint8_t *load_base = request_pages(&global_allocator, num_pages);
    if (!load_base)
    {
        free(file_buf);
        return;
    }

    for (size_t p = 0; p < num_pages; ++p)
    {
        void *virt = load_base + p * 0x1000;
        void *phys = load_base + p * 0x1000;
        map_memory(global_page_table_manager, virt, phys, true, true);
    }

    void *entry = load_elf_reloc(file_buf, (void *)load_base);

    free(file_buf);

    proccess_t *proc = malloc(sizeof(proccess_t));
    if (!proc)
    {
        return;
    }

    int last_slash = 0;
    for (int i = 0; i < strlen(path); ++i)
    {
        if (path[i] == '/')
            last_slash = i;
    }
    char *name = path + last_slash + 1;

    create_process(proc, name, entry);
    start_proc(proc);
}
