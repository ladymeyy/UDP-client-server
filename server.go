package main

import (
	"fmt"
	"net"
	//"time"
	"strings"
)

const port_in = ":5001"
const port_out = ":7777"


type typeBAddresses  struct {
    addrSlice []*net.UDPAddr
    lock  bool
}


func compareUDPAddr(a, b *net.UDPAddr) bool {
 	if &a == &b {
    	return true
  	}

  	if strings.Compare(a.IP.String(), b.IP.String()) != 0  {
    	return false
  	}
  	return true
}

func findClientIndex(clients *typeBAddresses, targetClient *net.UDPAddr) int{
	for i := range clients.addrSlice {
    	if compareUDPAddr(clients.addrSlice[i], targetClient) {
        	// found
        	return i
    	}
	}
	//not found
	return -1
}

func insertAddress(clients *typeBAddresses, targetClient *net.UDPAddr){
	if findClientIndex(clients, targetClient) != -1{
		fmt.Println("Client:", targetClient.IP.String() , " already connected ")
		return
	}else{
		for clients.lock { } //Don't writing while removing
		clients.lock= true
		clients.addrSlice = append(clients.addrSlice, targetClient)
		clients.lock=false
		fmt.Println("Inserted client:", targetClient.IP.String() ," to list ")
	}
}


func removeAddress(clients *typeBAddresses, targetClient *net.UDPAddr){
	var i int = findClientIndex(clients, targetClient)

    if i != -1 {
    	for clients.lock { } //Don't remove while writing
		clients.lock= true
        clients.addrSlice = append(clients.addrSlice[:i], clients.addrSlice[i+1:]...)
      	clients.lock=false
        fmt.Println("Removed client:", targetClient.IP.String() ," from list ")
        return
    }
	fmt.Println("Warning: can't disconnect clinet:", targetClient.IP.String() ," not connected")
}


func typeAHandler(clients *typeBAddresses){
 	var singleClient *net.UDPAddr

	serverAddrA, err := net.ResolveUDPAddr("udp", port_in)
	if err != nil {
		panic(err)
	}

	serverConnA, err := net.ListenUDP("udp", serverAddrA)
	if err != nil {
		panic(err)
	}

	defer serverConnA.Close()

	func() {
		for {

			buf := make([]byte, 1024*1024)
			n, addr, err := serverConnA.ReadFromUDP(buf)
			if err != nil {
				fmt.Println("Error: ", err)
			}

			if singleClient == nil {
				singleClient = addr
				fmt.Println("defined client A: ", singleClient.IP.String())
			}else if !compareUDPAddr(singleClient, addr){
				fmt.Println("Warning! server supports only one type A")
				continue
			}

			//debug: fmt.Println("recv: ", n, "bytes ","from ", singleClient.IP.String())

			if clients.addrSlice != nil && !clients.lock{
				for i := range clients.addrSlice {
					serverConnA.WriteToUDP(buf[0:n], clients.addrSlice[i])
				}
			}
		}
	}()

}

func typeBHandler(clients *typeBAddresses){

	serverAddrB, err := net.ResolveUDPAddr("udp", port_out)
	if err != nil {
		panic(err)
	}

	serverConnB, err := net.ListenUDP("udp", serverAddrB)
	if err != nil {
		panic(err)
	}

	defer serverConnB.Close()

	func() {
		for {

			buf := make([]byte, 1024)
			n, addr, err := serverConnB.ReadFromUDP(buf)

			//debug: fmt.Println("recv: ",string(buf[0:n]) ,n, "bytes ","from ", addr)

			if(strings.Compare(string(buf[0:n]),"CONNECT") == 0 ){
				insertAddress(clients, addr)
			}else if(strings.Contains(string(buf[0:n]),"DISCONNECT")){
				removeAddress(clients, addr)
			}
			if err != nil {
				fmt.Println("Error: ", err)
			}

		}
	}()
}

func main() {

	fmt.Println("Programming task server")

	//pointer to struct
	var clients = &typeBAddresses{lock: false}

	go typeAHandler(clients)
	fmt.Println("Waiting for type A (single) client to connect. Server port " + port_in)

	go typeBHandler(clients)
	fmt.Println("Waiting for type B clients to connect. Server port " + port_out)

	//blocking forever
	select {}
}
