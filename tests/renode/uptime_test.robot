*** Comments ***
Copyright (c) 2024, Victor Chavez
SPDX-License-Identifier: Apache-2.0

*** Variables ***
${UART}                       sysbus.uart0

*** Keywords ***
Create Machine
    [Arguments]              ${elf}

    Execute Command          mach create
    Execute Command          machine LoadPlatformDescription @platforms/cpus/nrf52840.rep
    Execute Command          sysbus LoadELF ${elf}
    Execute Command          connector Connect sysbus.radio wireless
    Execute Command          showAnalyzer sysbus.uart0
    Create Terminal Tester   sysbus.uart0   machine=central

*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Library    Process
Resource        ${RENODEKEYWORDS}

*** Test Cases ***

Uptime Demo
    Execute Command           emulation CreateIEEE802_15_4Medium "wireless"

    Execute Command           mach add "central"
    Execute Command           machine LoadPlatformDescription @platforms/cpus/nrf52840.repl
    Execute Command           sysbus LoadELF @${CURDIR}/ble_central/build/zephyr/zephyr.elf
    Execute Command           connector Connect sysbus.radio wireless

    Execute Command           showAnalyzer ${UART}
    ${cen_uart}=  Create Terminal Tester   ${UART}   machine=central

    Execute Command           mach add "peripheral"
    Execute Command           mach set "peripheral"
    Execute Command           machine LoadPlatformDescription @platforms/cpus/nrf52840.repl
    Execute Command           sysbus LoadELF @${CURDIR}/../../samples/uptime/build/zephyr/zephyr.elf
    Execute Command           connector Connect sysbus.radio wireless

    Execute Command           showAnalyzer ${UART}
    ${per_uart}=  Create Terminal Tester   ${UART}   machine=peripheral

    Execute Command           emulation SetGlobalQuantum "0.00001"

    Start Emulation

    Wait For Line On Uart     Booting Zephyr                                testerId=${cen_uart}
    Wait For Line On Uart     Booting Zephyr                                testerId=${per_uart}
    Wait For Line On Uart     Scanning successfully started                 testerId=${cen_uart}
    Wait For Line On Uart     Bluetooth initialized		                    testerId=${per_uart}
    Wait For Line On Uart     Matched Uptime adv. UUID                      testerId=${cen_uart}
    Wait For Line On Uart     Connected                		                testerId=${per_uart}
    Wait For Line On Uart     Connected                		                testerId=${cen_uart}
    Wait For Line On Uart     Service found                                 testerId=${cen_uart}
    Wait For Line On Uart     Chrc 3/3 found                                testerId=${cen_uart}
    Wait For Line On Uart     Subscribed                                    testerId=${cen_uart}
    Wait For Line On Uart     Characteristic Notify Uptime CCC changed      testerId=${per_uart}
    Wait For Line On Uart     Uptime value 1                                testerId=${cen_uart}
    Wait For Line On Uart     Uptime value 2                                testerId=${cen_uart}
    Wait For Line On Uart     Uptime value 3                                testerId=${cen_uart}