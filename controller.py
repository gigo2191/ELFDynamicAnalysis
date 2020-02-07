import os, sys, traceback, socket, datetime
import time
from pyVim import connect
from pyVim.task import WaitForTask
import ssl
import pyVmomi
from pyVmomi import vim
from configs import *
#import libvirt

class VMController:
	def __init__(self, host=HOST, port=PORT, user=PORT, password=PASSWORD):
		self.vs = None
		self.host = host
		self.port = port
		self.user = user
		self.password = password
		self.context = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
		self.ip = None
		self.getIP()

	def getIP(self):
		try:
			self.ip = socket.gethostbyname(socket.gethostname())
			print 'ip is %s' % self.ip
		except Exception as e:
			print "Cannot get local IP address : %s" % str(e)
			print traceback.print_exc()

	def connect(self):
		try:
			self.vs = connect.SmartConnect(host=self.host, port=self.port, user=self.user, pwd=self.password, sslContext=self.context)
			return True
		except Exception as e:
			print "Cannot connect to host : %s" % str(e)
			print traceback.print_exc()
			return False

	def disconnect(self):
		try:
			if self.vs != None:
				connect.Disconnect(self.vs)
		except Exception as e:
			print str(e)

	def reconnect(self):
		try:
			if self.vs != None:
				connect.Disconnect(self.vs)
			self.vs = connect.Connect(self.host, self.port, self.user, self.password)
			return True
		except Exception as e:
			print str(e)
			return False

	def getVM(self):
		try:
			if self.ip == None:
				return None
			searcher = self.vs.content.searchIndex
			vm = searcher.FindByIp(None, self.ip, vmSearch=True)
			print vm.config.name
			return vm
		except Exception as e:
			print str(e)
			print traceback.print_exc()
			return None


	def getVMByName(self, name):
		try:
			dc = self.vs.content.rootFolder.childEntity[0]
			searcher = self.vs.content.searchIndex
			vm = searcher.FindChild(dc.vmFolder, name)
			print vm.config.name
			return vm
		except Exception as e:
			print str(e)
			print traceback.print_exc()
			return None
	
	def resetVM(self, vm):
		try:
			t = vm.ResetVM_Task()
			WaitForTask(t)
			#self.wait(t.info)
		except Exception as e:
			print str(e)

	def revertCurrentSnapshotVM(self, vm):
		try:
			t = vm.RevertToCurrentSnapshot_Task()
			WaitForTask(t)
			#self.wait(t.info)
		except Exception as e:
			print str(e)

	def wait(self, taskInfo):
		while True:
			print taskInfo.state
			if taskInfo.state == "success":
				print "task : successed"
				return True
			if taskInfo.state == "error":
				print "task : error"
				return False
			time.sleep(1)

		
	def checkVM(self, vm):
		try:
			status = vm.guestHeartbeatStatus
			print status
			return status
		except Exception as e:
			print str(e)

	def main(self):
		while True:
			try:
				self.connect()
				vm = self.getVM()
				self.revertCurrentSnapshotVM(vm)
				self.disconnect()
			except Exception as e:
				print str(e)
				self.disconnect()
				time.sleep(10)

	def renewalSnapshot(self, name="", description=""):
		try:
			self.connect()
			vm = self.getVM()
			old = vm.snapshot.currentSnapshot
			name = name + str(datetime.date.today())
			description = description + str(datetime.datetime.now())

			t = vm.CreateSnapshot_Task(name, description, True, True)
			print t.info.state
			if t.info.state == "running":
				self.disconnect()
				return
			WaitForTask(t)
			print 'start remove'
			t = old.RemoveSnapshot_Task(False)
			WaitForTask(t)
			
			print 'finish remove'
			self.disconnect()
		except Exception as e:
			print str(e)
			self.disconnect()


if __name__ == "__main__":
	vc = VMController(HOST, PORT, USERNAME, PASSWORD)
	try:
		vc.connect()
		vm = vc.getVM()
		if vm == None: # when vm is powered off
			print i
			time.sleep(10)
			vm = vc.getVM(i)
			if vm == None:
				vm = vc.getVMByName(i)
				print 'reverting..'
				vc.revertCurrentSnapshotVM(vm)
				print 'reverted'
				time.sleep(10)
		vc.disconnect()
		time.sleep(10)
	except Exception as e:
		print e
		print traceback.print_exc()
		time.sleep(10)

