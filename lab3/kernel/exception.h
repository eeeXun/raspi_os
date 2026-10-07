unsigned long long irq_save();
void irq_restore(unsigned long long daif_state);
void enable_interrupt();
void disable_interrupt();
