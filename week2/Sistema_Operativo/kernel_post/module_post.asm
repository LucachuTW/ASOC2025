; Módulo post simple: firma MOD0 y mensaje de demostración
BITS 16
ORG 0

magic: db 'MOD0'
version: dw 0x0001
length:  dw 16
payload: db 'Hola desde MOD0', 0

; Relleno al sector
TIMES 512-($-$$) db 0
