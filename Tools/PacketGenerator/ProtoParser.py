class ProtoParser():
    """description of class"""
    # C++ 에서는 private으로 멤버변수를 만들어 놨는데
    # python은 생성자에서 runtime에 그걸 만듦

                                # C_, S_ 받기
    def __init__(self, start_id, recv_prefix, send_prefix):
        self.recv_pkt = [] # 수신 패킷 목록
        self.send_pkt = [] #송신 패킷 목록
        self.total_pkt = [] #모든 패킷 목록
        self.start_id = start_id
        self.id = start_id
        self.recv_prefix = recv_prefix
        self.send_prefix = send_prefix

    def parse_proto(self, path):
        #경로 받아서 파일 오픈
        f = open(path, 'r')
        lines = f.readlines() #한줄한줄 읽어서 lines에 배열로 저장

        for line in lines:
            if line.startswith('message') == False: #message로 시작하는 줄이라면 다음단어 체크
                continue

            pkt_name = line.split()[1].upper() #0번은 message, 1번이 패킷 이름, 그걸 대문자로 만들어서 비교
            if pkt_name.startswith(self.recv_prefix):
                self.recv_pkt.append(Packet(pkt_name, self.id))
            elif pkt_name.startswith(self.send_prefix):
                self.send_pkt.append(Packet(pkt_name, self.id))
            else: #패킷이 아닐 경우
                continue

            # 패킷인 경우니까 total에 다시 한번 등록
            self.total_pkt.append(Packet(pkt_name, self.id))
            self.id += 1

        f.close()

class Packet:
    def __init__(self, name, id):
        self.name = name
        self.id = id


