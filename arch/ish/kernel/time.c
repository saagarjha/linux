#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/sched_clock.h>
#include <linux/timekeeping.h>
#include <asm/delay.h>
#include <user/user.h>
#include "time_user.h"

// TODO make these make sense
void __const_udelay(unsigned long xloops)
{
	while (xloops--) {}
}
void calibrate_delay(void)
{
}

void read_persistent_clock64(struct timespec64 *ts)
{
	uint64_t unix_nanos = host_unix_nanos();
	set_normalized_timespec64(ts, unix_nanos / NSEC_PER_SEC,
				  unix_nanos % NSEC_PER_SEC);
}

static uint64_t host_clock_read(struct clocksource *cs)
{
	return host_monotonic_nanos();
}

static struct clocksource host_clocksource = {
	.name = "host",
	.rating = 300,
	.read = host_clock_read,
	.mask = CLOCKSOURCE_MASK(64),
	.flags = CLOCK_SOURCE_IS_CONTINUOUS | CLOCK_SOURCE_VALID_FOR_HRES,
};

static int host_timer_next_event(unsigned long ns, struct clock_event_device *ce)
{
	timer_set_next_event(smp_processor_id(), ns);
	return 0;
}
static int host_timer_state_shutdown(struct clock_event_device *ce)
{
	timer_set_next_event(smp_processor_id(), U64_MAX);
	return 0;
}
static int host_timer_state_oneshot(struct clock_event_device *ce)
{
	return 0;
}

static DEFINE_PER_CPU(struct clock_event_device, host_clockevents) = {
	.name = "host-timer",
	.rating = 300,
	.features = CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event = host_timer_next_event,
	.set_state_oneshot = host_timer_state_oneshot,
	.set_state_shutdown = host_timer_state_shutdown,
	.max_delta_ns = S64_MAX,
	.mult = 1,
};

static irqreturn_t timer_irq(int irq, void *dev_id)
{
	struct clock_event_device *dev = this_cpu_ptr(&host_clockevents);
	dev->event_handler(dev);
	return IRQ_HANDLED;
}

#define GHZ 1000000000

void __init local_timer_init(void) {
	struct clock_event_device *local_clockevents = this_cpu_ptr(&host_clockevents);
	BUG_ON(local_clockevents->cpumask != NULL);
	local_clockevents->cpumask = cpumask_of(smp_processor_id());
	clockevents_register_device(local_clockevents);
}

void __init time_init(void)
{
	int err;

	sched_clock_register(host_monotonic_nanos, 64, 1*GHZ);
	err = clocksource_register_hz(&host_clocksource, 1*GHZ);
	if (err) {
		pr_err("clocksource: failed to register host: %d\n", err);
	}

	irq_set_handler(TIMER_IRQ, handle_percpu_irq);
	err = request_irq(TIMER_IRQ, timer_irq, IRQF_TIMER, "timer", NULL);
	if (err) {
		pr_err("timer: failed to request_irq: %d\n", err);
		return;
	}
	timer_start_thread();
	local_timer_init();
}
