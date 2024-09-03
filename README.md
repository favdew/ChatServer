# C++ 채팅 서버   
boost/asio 라이브러리 기반의 채팅 서버   
데이터베이스로 MySQL을 사용하여 연동 라이브러리인 mysqlcppconn 을 사용   
암호화와 관련하여 CryptoPP 라이브러리를 사용   
클라이언트는 WinAPI 기반으로 작성   

   
# 실행시 주의사항   
client/connect_conf.conf 와 server/config/connection.conf 의 두 파일에 데이터베이스 접속 관련 정보를 설정해야 합니다.
