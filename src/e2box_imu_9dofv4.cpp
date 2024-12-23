#include "e2box_imu_9dofv4.h"

e2box_imu_9dofv4::e2box_imu_9dofv4()
{
    Initialize();
}

e2box_imu_9dofv4::~e2box_imu_9dofv4()
{
}

void e2box_imu_9dofv4::Initialize(void)
{
    m_dwordCounterChecksumPass = 0;
    m_dwordCounterChecksumFail = 0;

    memset(m_dQuaternion,0,sizeof(m_dQuaternion));
    memset(m_dAccel,0,sizeof(m_dAccel));
    memset(m_dAngRate,0,sizeof(m_dAngRate));

    //m_sPacket = "";

    SetStatusHeaderDetect(false);
    data_acquisition = false;
    SetStatusUpdateData(false);

    m_iRawDataIndex = 0;
}

void e2box_imu_9dofv4::ExtractData(unsigned char byteCurrent)
{
    // if(GetStatusHeaderDetect() == false){
    //     if(byteCurrent == 0x2A){	// mark *
    //         SetStatusHeaderDetect(true);
    //         memset(m_abyteRawData,0,sizeof(m_abyteRawData));
    //         m_abyteRawData[0] = byteCurrent;
    //         m_iRawDataIndex = 1;
    //     }
    // }
    // else{
    //     m_abyteRawData[m_iRawDataIndex++] = byteCurrent;
    //     if(byteCurrent == 0x0D){ // 0x0d = CR, 0x0A = LF
    //         for(int i = 0; i < m_iRawDataIndex; i++) m_acCopiedRawData[i] = m_abyteRawData[i];
    //         data_acquisition = true;
    //         SetStatusHeaderDetect(false);
    //     }
    //     if(m_iRawDataIndex > 90) SetStatusHeaderDetect(false);
    // }
    if(byteCurrent == '\n'){
        // '\n'을 만나면 패킷 수신 완료
        m_acCopiedRawData[m_iRawDataIndex] = '\0'; // 문자열 종료
        data_acquisition = true;
        m_iRawDataIndex = 0; // 인덱스 초기화
    }
    else
        m_acCopiedRawData[m_iRawDataIndex++] = byteCurrent;
}

bool e2box_imu_9dofv4::HandlingDataIMU(){
    // if(GetStatusHeaderDetect() == false){
    //     InterpretGeneral();
    //     SetStatusUpdateData(true);
    //     m_dwordCounterChecksumPass++;
    // }
    // else {
    //     m_dwordCounterChecksumFail++;
    //     return false;
    // }
    InterpretGeneral();
    return true;
}

void e2box_imu_9dofv4::InterpretGeneral(void)
{
    char chId[10];
    double dataValues[20];

    const char* format = "%[^ \t\n\r,]%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf%*[, ]%lf";
    // sscanf를 사용하여 데이터 파싱
    int parsedItems = sscanf(m_acCopiedRawData, format,
                             chId,
                             &dataValues[0], &dataValues[1], &dataValues[2],
                             &dataValues[3], &dataValues[4], &dataValues[5],
                             &dataValues[6], &dataValues[7], &dataValues[8],
                             &dataValues[9], &dataValues[10]);

    m_dQuaternion[0] = dataValues[0];
    m_dQuaternion[1] = dataValues[1];
    m_dQuaternion[2] = dataValues[2];
    m_dQuaternion[3] = dataValues[3];

    m_dAngRate[0] = dataValues[4];
    m_dAngRate[1] = dataValues[5];
    m_dAngRate[2] = dataValues[6];

    m_dAccel[0] = dataValues[7];
    m_dAccel[1] = dataValues[8];
    m_dAccel[2] = dataValues[9];

    // cout << chId << ", " << *m_dQuaternion << ", " << *m_dAngRate << ", " << *m_dAccel << endl;
}

BOOL e2box_imu_9dofv4::CalcCheckSum(void)
{
    sscanf(&m_acCopiedRawData[m_iRawDataIndex-3],"%x",&iChecksum);

    for(int i = 1; i < m_iRawDataIndex - 5; i++){
        if(i == 1) iCandisum = CheckBYTEXOR(m_acCopiedRawData[i], m_acCopiedRawData[i+1]);
        else iCandisum = CheckBYTEXOR(iCandisum, m_acCopiedRawData[i+1]);
    }

    if(iCandisum == iChecksum) return true;
    else return false;
}

DWORD e2box_imu_9dofv4::CheckBYTEXOR(BYTE byteCompared1, BYTE byteCompared2)
{
    BYTE byteMask = 0x80;
    BYTE byteResult = 0;

    for(int i = 0; i < 8; i++){
        byteResult <<= 1;

        if((byteCompared1 & byteMask) == (byteCompared2 & byteMask)) byteResult |= 0x0;
        else byteResult |= 0x1;

        byteMask >>= 1;
    }

    return byteResult;
}


