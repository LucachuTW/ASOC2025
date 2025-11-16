#!/usr/bin/env sh
# generate_module_info.sh
# Usage: generate_module_info.sh [MODULE_LOAD_ADDRESS]

DEFAULT_ADDR="$1"
# choose module source file
src=""
if [ -f module_post.img ]; then src=module_post.img
elif [ -f module_post.bin ]; then src=module_post.bin
elif [ -f module_post.elf ]; then src=module_post.elf
fi

mkdir -p kernel_boot
if [ -z "$src" ]; then
  # no module
  printf '%s\n' '#ifndef MODULE_INFO_H' '#define MODULE_INFO_H' '#define MODULE_SIZE_BYTES 0' '#define MODULE_SECTORS_COUNT 0' "#define MODULE_LOAD_ADDRESS $DEFAULT_ADDR" '#endif' > kernel_boot/module_info.h
  echo "module_info.h: no module found, wrote zeros (MODULE_LOAD_ADDRESS=$DEFAULT_ADDR)"
  exit 0
fi

size=$(stat -f%z "$src" 2>/dev/null || stat -c%s "$src")
sectors=$(( (size + 511) / 512 ))
addr="$DEFAULT_ADDR"
if [ -f kernel_padded.bin ] && [ "$addr" = "$DEFAULT_ADDR" ]; then
  kparts=$(stat -f%z kernel_padded.bin 2>/dev/null || stat -c%s kernel_padded.bin)
  kpages=$(( (kparts + 4095) / 4096 ))
  base=$(( 0x200000 + kpages * 4096 ))
  addr=$(printf '0x%06x' $base)
fi

# write file
printf '%s\n' '#ifndef MODULE_INFO_H' '#define MODULE_INFO_H' > kernel_boot/module_info.h
printf '#define MODULE_SIZE_BYTES %d\n' $size >> kernel_boot/module_info.h
printf '#define MODULE_SECTORS_COUNT %d\n' $sectors >> kernel_boot/module_info.h
printf '#define MODULE_LOAD_ADDRESS %s\n' $addr >> kernel_boot/module_info.h
printf '%s\n' '#endif' >> kernel_boot/module_info.h

echo "module_info.h: source=$src size=$size bytes sectors=$sectors (MODULE_LOAD_ADDRESS=$addr)"
