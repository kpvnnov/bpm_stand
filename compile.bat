@echo off
iar\bin\icc430.exe -DCABLE -ODebug\Obj\ -e -K -gA -s9 -RCODE -r0 -LDebug\List\ -q -T -t8 -ADebug\List\ -Iiar\inc\ uart.c
iar\bin\icc430.exe -DCABLE -ODebug\Obj\ -e -K -gA -s9 -RCODE -r0 -LDebug\List\ -q -T -t8 -ADebug\List\ -Iiar\inc\ adc.c
iar\bin\icc430.exe -DCABLE -ODebug\Obj\ -e -K -gA -s9 -RCODE -r0 -LDebug\List\ -q -T -t8 -ADebug\List\ -Iiar\inc\ msp_main.c
iar\bin\icc430.exe -DCABLE -ODebug\Obj\ -e -K -gA -s9 -RCODE -r0 -LDebug\List\ -q -T -t8 -ADebug\List\ -Iiar\inc\ timer_a.c

rem #       Command line  =  -OE:\work\msp\Debug\Obj\ -e -K -gA -s9 -RCODE -r0   #
rem #                        -LE:\work\msp\Debug\List\ -q -T -t8                 #
rem #                        -AE:\work\msp\Debug\List\                           #
rem #                        -IF:\Documents and Settings\peter_safe\msp\IAR Systems\ew23\430\inc\ #
rem #                        E:\work\msp\msp_main.c                              #
