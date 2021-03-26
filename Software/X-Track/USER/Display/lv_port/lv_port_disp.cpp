#include "../Display.h"

static lv_disp_drv_t* disp_drv_p;

#define DISP_BUF_SIZE        (LV_HOR_RES_MAX * LV_VER_RES_MAX)
static lv_color_t lv_full_disp_buf[DISP_BUF_SIZE];
static lv_color_t* lv_disp_buf1 = lv_full_disp_buf;

#define DISP_DMA_Channel         DMA1_Channel3
#define DISP_DMA_MAX_SIZE        65535
static uint8_t* disp_dma_tar_p = NULL;
static uint8_t* disp_dma_cur_p = NULL;

/**
  * @brief  ʹ��DMA���ͻ���������
  * @param  buf:��������ַ
  * @param  size:����������
  * @retval ��
  */
static void disp_spi_dma_send(const void* buf, uint32_t size)
{
    if(size > DISP_DMA_MAX_SIZE)
    {
        if(disp_dma_tar_p == NULL)
        {
            disp_dma_tar_p = (uint8_t*)buf + size;
        }
        disp_dma_cur_p = (uint8_t*)buf + DISP_DMA_MAX_SIZE;
        size = DISP_DMA_MAX_SIZE;
    }
    else
    {
        disp_dma_cur_p = NULL;
        disp_dma_tar_p = NULL;
    }
    
    DMA_ChannelEnable(DISP_DMA_Channel, DISABLE);
    DISP_DMA_Channel->CMBA = (uint32_t)buf;
    DISP_DMA_Channel->TCNT = size;
    DMA_ChannelEnable(DISP_DMA_Channel, ENABLE);
}

/**
  * @brief  ��Ļˢ�»ص�����
  * @param  disp:��Ļ������ַ
  * @param  area:ˢ������
  * @param  color_p:ˢ�»�������ַ
  * @retval ��
  */
static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    SCREEN_CLASS* screen = (SCREEN_CLASS*)disp->user_data;
    
    disp_drv_p = disp;

//    screen->drawFastRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
//    lv_disp_flush_ready(disp_drv_p);
//    return;

    const int16_t w = (area->x2 - area->x1 + 1);
    const int16_t h = (area->y2 - area->y1 + 1);
    const uint32_t size = w * h * sizeof(lv_color_t);

    /*����ˢ������*/
    screen->setAddrWindow(area->x1, area->y1, area->x2, area->y2);

    /*����ģʽ*/
    digitalWrite_LOW(SCREEN_CS_PIN);
    digitalWrite_HIGH(SCREEN_DC_PIN);

    /*DMA��������*/
    disp_spi_dma_send(color_p, size);
}

extern "C" {
    void DMA1_Channel3_IRQHandler(void)
    {
        /*DMA��������ж�*/
        if(DMA_GetITStatus(DMA1_INT_TC3) != RESET)
        {
            DMA_ClearITPendingBit(DMA1_INT_TC3);
            if(disp_dma_cur_p < disp_dma_tar_p)
            {
                disp_spi_dma_send(disp_dma_cur_p, disp_dma_tar_p - disp_dma_cur_p);
            }
            else
            {
                digitalWrite_HIGH(SCREEN_CS_PIN);
                lv_disp_flush_ready(disp_drv_p);
            }
        }
    }
}

/**
  * @brief  DMA��ʼ��
  * @param  ��
  * @retval ��
  */
static void disp_spi_dma_init()
{
    DMA_InitType  DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPERIPH_DMA1, ENABLE);

    DMA_Reset(DISP_DMA_Channel);

    DMA_DefaultInitParaConfig(&DMA_InitStructure);
    
    DMA_InitStructure.DMA_Mode = DMA_MODE_NORMAL;  //��������������ģʽ
    DMA_InitStructure.DMA_MTOM = DMA_MEMTOMEM_DISABLE;  //DMAͨ�� û������Ϊ�ڴ浽�ڴ洫��
    DMA_InitStructure.DMA_Direction = DMA_DIR_PERIPHERALDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
    DMA_InitStructure.DMA_Priority = DMA_PRIORITY_MEDIUM; //DMAͨ�� �����ȼ�
    
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)lv_disp_buf1;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_MemoryInc = DMA_MEMORYINC_ENABLE;  //�ڴ��ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryDataWidth = DMA_MEMORYDATAWIDTH_BYTE; //���ݿ��
    DMA_InitStructure.DMA_BufferSize = DISP_DMA_MAX_SIZE;  //DMAͨ����DMA����Ĵ�С
    
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI1->DT));  //DMA����SPI����ַ
    DMA_InitStructure.DMA_PeripheralInc = DMA_PERIPHERALINC_DISABLE;  //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataWidth = DMA_PERIPHERALDATAWIDTH_BYTE;  //���ݿ��

    DMA_Init(DISP_DMA_Channel, &DMA_InitStructure);

    SPI_I2S_DMAEnable(SPI1, SPI_I2S_DMA_TX, ENABLE);

    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    
    DMA_INTConfig(DISP_DMA_Channel, DMA_INT_TC, ENABLE);
}

static void disp_wait_cb(lv_disp_drv_t* disp_drv)
{
    __wfi();
}

/**
  * @brief  ��Ļ��ʼ��
  * @param  ��
  * @retval ��
  */
void lv_port_disp_init(SCREEN_CLASS* scr)
{
    disp_spi_dma_init();

    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, lv_disp_buf1, NULL, DISP_BUF_SIZE);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.wait_cb = disp_wait_cb;
    disp_drv.buffer = &disp_buf;
    disp_drv.user_data = scr;
    lv_disp_drv_register(&disp_drv);
}
