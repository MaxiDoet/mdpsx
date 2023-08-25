#ifndef _dma_h
#define _dma_h

#define DMA_DICR_IRQ_MASTER_ENABLE  (1 << 23)
#define DMA_DICR_IRQ_MASTER         (1 << 31)

#define DMA_CHANNEL_MDEC_IN     0
#define DMA_CHANNEL_MDEC_OUT    1
#define DMA_CHANNEL_GPU         2
#define DMA_CHANNEL_CDROM       3
#define DMA_CHANNEL_SPU         4
#define DMA_CHANNEL_PIO         5
#define DMA_CHANNEL_OTC         6

#define DMA_CHANNEL_REG_MADR            0x0
#define DMA_CHANNEL_REG_BCR             0x4
#define DMA_CHANNEL_REG_CHCR            0x8

#define DMA_CHANNEL_CHCR_DIRECTION  (1 << 0)
#define DMA_CHANNEL_CHCR_ADDR_STEP  (1 << 1)
#define DMA_CHANNEL_CHCR_START_BUSY (1 << 24)
#define DMA_CHANNEL_CHCR_TRIGGER    (1 << 28)

typedef struct dma_channel_state_t {
    uint32_t madr;
    uint32_t bcr;
    uint32_t chcr;
} dma_channel_state_t;

typedef struct dma_state_t {
    uint32_t dpcr;
    uint32_t dicr;

    dma_channel_state_t channels[7];
} dma_state_t;

/*
void dma_write(dma_state_t *dma_state, uint32_t addr, uint32_t value);
uint32_t dma_read(dma_state_t *dma_state, uint32_t addr);
*/

#endif