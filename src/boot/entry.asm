; SPDX-License-Identifier: GPL-3.0-only
; Copyright (C) 2026 Retro-Orca

    cpu SH7600
    supmode on
    padding on
    org $06002E80
    mov.l #$06004000,r0
    jmp @r0
    nop
    ltorg
    end
