import os
import sys
import subprocess
import time
import traceback
import shutil
from threading import Timer
from configs import *
from controller import *

def check_version():
	# update code and take snapshot.
	try:
		vc = VMController(HOST, PORT, USERNAME, PASSWORD)
		vc.renewalSnapshot()
	except Exception as e:
		print e
		print traceback.print_exc()
		return False
	return True

class Controller:
	def __init__(self):
		self.sha256 = ""
		self.request_id = ""
		self.task = None
		self.mtimer = None
		self.ptimer = None
		self.timeout = 60
		self.target = ""
		self.malware = None
		self.analyzer = None
		self.tcpdump = None
		self.running = False

	def download(self, sha256):
		self.target = SAMPLE_PATH
		os.system("chmod 777 %s" % self.target)
		print "Download Sample."
	def clean_dir(self):
		try:
			shutil.rmtree(RESULT_PATH)
		except Exception as e:
			print e
			print "Cannot Clean result directory."

	def get_task(self):
		print "Get Pending Task."

	def report_task(self):
		print "report Task."

	def upload_results(self):
		print "Upload result files."

	def run_analyzer(self, ):
		cmd = "%s %s %s %s" % (ANALYZER, STRACE_PATH, PCAP_PATH, RESULT_PATH)
		self.analyzer = subprocess.Popen("exec " + cmd, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
		print cmd
		
	def run_sample(self):
		cmd = "strace -f -ttt -o %s %s" % (STRACE_PATH, self.target)
		self.malware = subprocess.Popen("exec " + cmd, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
		print cmd

	def run_tcpdump(self):
		cmd = "/usr/sbin/tcpdump -U -q -s 0 -n -i ens33 -w %s" % PCAP_PATH
		self.tcpdump = subprocess.Popen("exec " + cmd, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
		print cmd

	def terminate_sample(self):
		if self.malware != None:
			try:
				self.malware.send_signal(subprocess.signal.SIGKILL)
				print "Terminate Sample."
			except Exception as e:
				print e

	def terminate_analyzer(self):
		if self.analyzer != None:
			try:
				self.analyzer.send_signal(subprocess.signal.SIGKILL)
				print "Terminate Analyzer."
			except Exception as e:
				print "Analyzer was terminated already."

	def terminate_tcpdump(self):
		if self.tcpdump != None:
			try:
				self.tcpdump.send_signal(subprocess.signal.SIGKILL)
				print "Terminate Tcpdump."
			except Exception as e:
				print e

	def set_mtimer(self):
		self.mtimer = Timer(self.timeout, self.mtimeout)
		self.mtimer.start()

	def set_ptimer(self):
		self.ptimer = Timer(self.timeout, self.ptimeout)
		self.ptimer.start()

	def complete(self):
		self.mtimer.cancel()
		self.terminate_sample()
		self.terminate_tcpdump()
		self.running = False

	def mtimeout(self):
		self.complete()

	def ptimeout(self):
		print "Reporting Time Out."
		self.report()

	def start(self):
		self.running = True
		self.set_mtimer()
		self.run_tcpdump()
		self.run_sample()

	def process(self):
		self.set_ptimer()
		self.run_analyzer()

	def report(self):
		self.ptimer.cancel()
		self.terminate_analyzer()
		self.report_task()
		self.upload_results()

	def wait_analyzer(self):
		try:
			while True:
				if self.analyzer.poll() != None:
					break
				time.sleep(1)
		except Exception as e:
			print e
			print traceback.print_exc()
		self.report()

	def wait_logging(self):
		try:
			while True:
				if not self.running:
					break
				time.sleep(1)
		except Exception as e:
			print e
			print traceback.print_exc()

	def main(self):
		try:
			self.clean_dir()
			self.download("")
			self.start()
			self.wait_logging()
			print "start processing.."
			self.process()
			self.wait_analyzer()
			print "finished"

		except Exception as e:
			print e
			print traceback.print_exc()

		try:
			vc = VMController(HOST, PORT, USERNAME, PASSWORD)
			vc.main()
		except Exception as e:
			print e
			print traceback.print_exc()


if __name__ == "__main__":
		check_version()
		c = Controller()
		c.main()
