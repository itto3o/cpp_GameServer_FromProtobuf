import argparse #프로그램 실행 시 인자들 파싱하기 위한 모듈
import jinja2
import ProtoParser

def main() :
    arg_parser = argparse.ArgumentParser(description = 'PacketGenerator')

    #default = ''이면 뒤에 오는 경로에 넘겨줄 것이다, 만들어줄 클래스의 이름 지정
    # 다른 이름을 해주고 싶다고 하면 인자로 받아주면 됨
    # help는 help를 칠 때 나오는 말
                                                    # proto파일이 있는 곳
    arg_parser.add_argument('--path', type=str, default='C:/Users/seoji/portfolio/CppGameServer/Server/Common/Protobuf/bin/Protocol.proto', help='proto path')
    arg_parser.add_argument('--output', type=str, default='TestPacketHandler', help='output file')
    arg_parser.add_argument('--recv', type=str, default='C_', help='recv convention')
    arg_parser.add_argument('--send', type=str, default='S_', help='send convention')
    args = arg_parser.parse_args() # 위에 애들, 인자들이 파싱이 돼서 사용 가능

    # proto파일 분석, message가 나오면 C_, S_로 시작하면 패킷으로 인정해서 추적, 메모리 상에서 들고 있을 것
    #-=> 클래스 새로 만들기 우클릭>새파일>클래스
    # 작성한 모듈을 import한 후 생성자로 만들기, 패킷을 분류함
    parser = ProtoParser.ProtoParser(1000, args.recv, args.send)
    parser.parse_proto(args.path)

    # jinja2 : 템플릿 엔진
    # 자동화되는 부분만 바꿔치기 하고 싶을 때
    # PacketGenerator 우클릭>추가>새폴더>Templates, templates안에 PacketHandler.h 만든 후
    # Server에 만든 ServerPacketHandler.h의 내용을 모두 복붙
    # 자동화 부분에 파이썬 코드를 섞어 줄 수 있다는 게 가장 큰 장점
    # asp.net, laser?기법도 템플릿을 사용한 것
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader = file_loader)

    # 샘플 코드, PacketHandler.h를 건네줌
    template = env.get_template('PacketHandler.h')
                            # parser객체를 통으로넘겨준다
                            # 왼쪽이 안에서 사용할 이름, 실제적으로 넘겨준게 오른쪾
                            # ==> PacketHandler.h에서 parser를 사용할 수 있게 됨
    output = template.render(parser=parser, output=args.output) # 실제코드와 메모리를 연동
    # 이후 PacketHandler.h 수정
                        
    # 최종 결과물을 파일에 저장
    f = open(args.output+'.h', 'w+')
    f.write(output) # 실제코드와 메모리를 연동)
    f.close()

    print(output)

    return

if __name__ == '__main__':
    main()
