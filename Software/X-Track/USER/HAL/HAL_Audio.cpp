#include "HAL.h"
#include "TonePlayer/TonePlayer.h"

static TonePlayer player;

/*����Ƶ�ʶ�Ӧ��*/
typedef enum
{
    L1 = 262,
    L1h = 277,
    L2 = 294,
    L2h = 311,
    L3 = 330,
    L4 = 349,
    L4h = 370,
    L5 = 392,
    L5h = 415,
    L6 = 440,
    L6h = 466,
    L7 = 494,
    M1 = 523,
    M1h = 554,
    M2 = 587,
    M2h = 622,
    M3 = 659,
    M4 = 698,
    M4h = 740,
    M5 = 784,
    M5h = 831,
    M6 = 880,
    M6h = 932,
    M7 = 988,
    H1 = 1046,
    H1h = 1109,
    H2 = 1175,
    H2h = 1245,
    H3 = 1318,
    H4 = 1397,
    H4h = 1480,
    H5 = 1568,
    H5h = 1661,
    H6 = 1760,
    H6h = 1865,
    H7 = 1976,
} Tone_TypeDef;

static void Tone_Callback(uint32_t freq, uint16_t volume)
{
    HAL::Buzz_Tone(freq);
}

/*������*/
static const TonePlayer::MusicNode_t Music_StartUp[] =
{
    {M1, 80},
    {M6, 80},
    {M3, 80},
};

/*�ػ���*/
static const TonePlayer::MusicNode_t Music_Shutdown[] =
{
    {M3, 80},
    {M6, 80},
    {M1, 80},
};

/*������ʾ��*/
static const TonePlayer::MusicNode_t Music_Error[] =
{
    {100, 80},
    {0,   80},
    {100, 80},
};

/*���ӳɹ���*/
static const TonePlayer::MusicNode_t Music_Connect[] =
{
    {H1, 80},
    {H2, 80},
    {H3, 80},
};

/*�Ͽ�������*/
static const TonePlayer::MusicNode_t Music_Disconnect[] =
{
    {H3, 80},
    {H2, 80},
    {H1, 80},
};

/*�źŲ��ȶ���ʾ��*/
static const TonePlayer::MusicNode_t Music_UnstableConnect[] =
{
    {H1, 80},
    {0, 80},
    {H1, 80},
};

/*���������ʾ��*/
static const TonePlayer::MusicNode_t Music_BattChargeStart[] =
{
    {L1, 80},
    {L3, 80},
};

/*���ر���ʾ��*/
static const TonePlayer::MusicNode_t Music_BattChargeEnd[] =
{
    {L3, 80},
    {L1, 80},
};

/*�豸������ʾ��*/
static const TonePlayer::MusicNode_t Music_DeviceInsert[] =
{
    /*C4,A3,F3,F4*/
    {M1, 180},
    {L6, 80},
    {L4, 80},
    {M4, 160},
};

/*�豸�γ���ʾ��*/
static const TonePlayer::MusicNode_t Music_DevicePullout[] =
{
    /*A4,F4,E4*/
    {L6, 80},
    {L4, 80},
    {L3, 80},
};

/*�޲�����ʾ��*/
static const TonePlayer::MusicNode_t Music_NoOperationWarning[] = {
    {4000, 40},
    {0, 80},
    {4000, 40},
    {0, 80},
    {4000, 40},
};

/*�б��������Ͷ���*/
typedef struct
{
    const TonePlayer::MusicNode_t* mc;
    uint16_t length;
    const char* name;
} MusicList_t;

#define ADD_MUSIC(mc) {Music_##mc,__Sizeof(Music_##mc), #mc}

/*��ʾ����ַ�����б�*/
static const MusicList_t MusicList[] =
{
    ADD_MUSIC(StartUp),
    ADD_MUSIC(Shutdown),
    ADD_MUSIC(Error),
    ADD_MUSIC(Connect),
    ADD_MUSIC(Disconnect),
    ADD_MUSIC(UnstableConnect),
    ADD_MUSIC(BattChargeStart),
    ADD_MUSIC(BattChargeEnd),
    ADD_MUSIC(DeviceInsert),
    ADD_MUSIC(DevicePullout),
    ADD_MUSIC(NoOperationWarning),
};

void HAL::Audio_Init()
{
    player.SetCallback(Tone_Callback);
}

void HAL::Audio_Update()
{
    player.Update(millis());
}

bool HAL::Audio_PlayMusic(const char* name)
{
    bool retval = false;
    for(int i = 0; i < __Sizeof(MusicList); i++)
    {
        if(strcmp(name, MusicList[i].name) == 0)
        {
            player.Play(MusicList[i].mc, MusicList[i].length);
            retval = true;
            break;
        }
    }
    return retval;
}
