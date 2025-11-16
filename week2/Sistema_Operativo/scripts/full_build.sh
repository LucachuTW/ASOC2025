#!/usr/bin/env sh
# full_build.sh - run 2-phase build until MODULE_LOAD_ADDRESS stabilizes (max 3 iterations)

set -e
DEFAULT_ADDR="$1"
if [ -z "$DEFAULT_ADDR" ]; then
  DEFAULT_ADDR="0x00190000"
fi

# Helper to compute address based on kernel_padded.bin
compute_addr() {
  if [ ! -f kernel_padded.bin ]; then
    echo "$DEFAULT_ADDR"
    return
  fi
  kparts=$(stat -f%z kernel_padded.bin 2>/dev/null || stat -c%s kernel_padded.bin)
  kpages=$(( (kparts + 4095) / 4096 ))
  base=$(( 0x200000 + kpages * 4096 ))
  printf '0x%06x' $base
}

# Phase 1: build kernel and module using default address
make kernel.bin
make kernel_padded.bin
addr=$(compute_addr)
export MODULE_LOAD_ADDRESS="$addr"
printf 'Phase1: MODULE_LOAD_ADDRESS=%s\n' "$addr"
make module_post.elf
make module_post.img
./scripts/generate_module_info.sh "$addr"
make -B kernel.elf

# Iterate up to 3 times to reach a stable address
MAX=3
i=0
while [ $i -lt $MAX ]; do
  i=$((i+1))
  new_addr=$(compute_addr)
  if [ "$new_addr" = "$addr" ]; then
    printf 'Address stable after %d iterations: %s\n' $i "$addr"
    break
  fi
  printf 'Address changed: %s -> %s; relinking module & regen info\n' "$addr" "$new_addr"
  addr="$new_addr"
  export MODULE_LOAD_ADDRESS="$addr"
  make module_post.elf
  make module_post.img
  ./scripts/generate_module_info.sh "$addr"
  make -B kernel.elf
done

# Finalize image
make os-image.bin
printf 'Full build complete: MODULE_LOAD_ADDRESS=%s\n' "$addr"
