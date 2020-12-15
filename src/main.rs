#![no_std]
#![no_main]


use esp8266_hal::prelude::*;
use esp8266_hal::target::Peripherals;
use panic_halt as _;
use core::fmt::Write;


#[entry]
fn main() -> ! {
    let dp = Peripherals::take().unwrap();
    let pins = dp.GPIO.split();

    // setup LED
    let mut led = pins.gpio2.into_push_pull_output();
    let (mut timer1, _) = dp.TIMER.timers();
    led.set_high().unwrap();

    // setup Serial
    let mut serial = dp.UART0.serial(pins.gpio1.into_uart(), pins.gpio3.into_uart());

    write!(serial, "\r\nStart Blinking!\r\n").unwrap();

    loop {
        // blink a few times in quick succession
        for n in (1..=40).rev() {
            led.toggle().unwrap();
            timer1.delay_ms(n*25);
            led.toggle().unwrap();
            timer1.delay_ms(n*25);
        
            // write something to serial
            let byte = nb::block!(serial.read()).unwrap();
            serial.write(byte).unwrap();
        }
    }
}

