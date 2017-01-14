#!/usr/bin/python

import sys
sys.path.append('../common')

import os
import json
import unittest
import requests
import hashlib
import time
import random
from threading import Thread
from util import check_output, check_both, run_bg

NETSIM = '../netsim/netsim.py'
VIDEO_SERVER_NAME = 'video.cs.cmu.edu'
PROXY = '../../handin/proxy'
LARGE_FOLDER = '/var/www/vod/large'

class Project3Test(unittest.TestCase):
    def __init__(self):
        self.topo_dir = "./topos/one-client"
        self.exc_info = []

    ########### SETUP/TEARDOWN ##########

    # Run once per test suite
    @classmethod
    def setUpClass(cls):
        pass

    # Run once per test suite
    @classmethod
    def tearDownClass(cls):
        pass

    # Run once per test
    def setUp(self):
        check_both('killall -9 proxy', False, False)
        check_both('killall -9 nameserver', False, False)
        self.start_netsim()
        self.proxyport1 = random.randrange(1025, 60000)
        self.proxyport2 = random.randrange(1025, 60000)
        self.dnsport = random.randrange(1025, 60000)

    # Run once per test
    def tearDown(self):
        check_both('killall -9 proxy', False, False)
        check_both('killall -9 nameserver', False, False)
        self.stop_netsim()


    ########## HELPER FUNCTIONS ##########

    def run_proxy(self, log, alpha, listenport, fakeip, dnsip, dnsport, serverip=''):
        check_both('rm %s' % log, False, False)
        run_bg('%s %s %s %s %s %s %s %s'\
            % (PROXY, log, alpha, listenport, fakeip, dnsip, dnsport, serverip))

    def run_events(self, events_file=None, bg=False):
        cmd = '%s %s run' % (NETSIM, self.topo_dir)
        if events_file:
            cmd += ' -e %s' % events_file
        if bg:
            run_bg(cmd)
        else:
            check_output(cmd)

    def start_netsim(self):
        if self.topo_dir:
            check_output('%s %s start' % (NETSIM, self.topo_dir))
    
    def stop_netsim(self):
        if self.topo_dir:
            check_output('%s %s stop' % (NETSIM, self.topo_dir))

    # Returns log entries as lists, one at a time. Use in for loop,
    # e.g., "for entry in iter_log(file_path):".
    def iter_log(self, log_file):
        with open(log_file, 'r') as logf:
            for line in logf:
                line = line.strip()
                if line:
                    yield line.split()
        logf.closed

    def get(self, url):
        return self.get_curl(url)

    def get_requests(self, url):
        return requests.get(url).content

    def get_curl(self, url):
        return check_both("curl -f -s %s" % url, shouldPrint=False)[0][0] 
        

    def check_gets(self, ip, port, num_gets, log_file, link_bw, expect_br, use=5, alpha=1.0, tput_margin=0.3, bitrate_margin=0.1, large=False):
        # TODO: better way to do this?
        if use == -1:
            use = 5

        if large:
            HASH_VALUE = {500: 'b1931364d7933ae90da7c6de423faf51b81503f4dfeb04da4be53dfb980c671e'}
        else:
            HASH_VALUE = {500: 'af29467f6793789954242d0430ce25e2fd2fc3a1aac5495ba7409ab853b1cdfa', 1000: 'f1ee215199d6c495388e2ac8470c83304e0fc642cb76fffd226bcd94089c7109'}
        

        # send a few gets (until we think their estimate should have stabilized)
        try: # this try is here so an exception will be thrown (and saved in self.exc_info) if we can't connect to proxy
            if large:
                content = self.get('http://%s:%s/vod/large/big_buck_bunny.f4m' % (ip, port))
            else:
                content = self.get('http://%s:%s/vod/big_buck_bunny.f4m' % (ip, port))

            for i in xrange(num_gets):
                if large:
                    content = self.get('http://%s:%s/vod/large/1000Seg2-Frag3' %(ip, port))
                else:
                    content = self.get('http://%s:%s/vod/1000Seg2-Frag7' %(ip, port))
            # check what bitrate they're requesting
            tputs = []
            tput_avgs = []
            bitrates = []
            for entry in self.iter_log(log_file):
                tputs.append(float(entry[2]))
                tput_avgs.append(float(entry[3]))
                bitrates.append(int(float(entry[4])))
            tputs = tputs[-use:]
            tput_avgs = tput_avgs[-use:]
            bitrates = bitrates[-use:]
            tput = float(sum(tputs))/len(tputs)
            tput_avg = float(sum(tput_avgs))/len(tput_avgs)
            bitrate = float(sum(bitrates))/len(bitrates)
            print tput, tput_avg, bitrate
        except Exception, e:
            self.exc_info = sys.exc_info()
            return

        print 'STATS: tput=%g, tput_avg=%g, bitrate=%g, expect_br=%g, link_bw=%g'\
            % (tput, tput_avg, bitrate, expect_br, link_bw)

        try: 
            self.assertTrue(abs(tput - link_bw) < tput_margin*link_bw)
            self.assertTrue(abs(tput_avg - link_bw) < (1.0/float(alpha))*tput_margin*link_bw)
            self.assertTrue(abs(bitrate - expect_br) < (1.0/float(alpha))*bitrate_margin*expect_br)

            # check the hash of the last chunk we requested
            chunkhash = hashlib.sha256(content).hexdigest()
            print 'Hash of last chunk: %s' % chunkhash
            self.assertTrue(chunkhash == HASH_VALUE[expect_br])
        except Exception, e:
            self.exc_info = sys.exc_info()

    def check_errors(self):
        if self.exc_info:
            raise self.exc_info[1], None, self.exc_info[2]


    def get_log_switch_len(self, log, num_trials, start_br, end_br):
        entries = [e for e in self.iter_log(log)]
        entries = entries[num_trials:]
        switch = 0
        for i,e in enumerate(entries):
            if float(e[4]) == end_br and switch == 0:
                switch = i
            if float(e[4]) == start_br:
                switch = 0
        return switch

    def print_log(self, log):
        print '\n\n#################### %s ####################' % log
        check_output('cat %s' % log)
        print '\n'



    ########### TEST CASES ##########

    def test_proxy_simple(self):
        PROXY_LOG = 'proxy.log'
        self.run_proxy(PROXY_LOG, '1', self.proxyport1, '1.0.0.1', '0.0.0.0', '0', '2.0.0.1')
        self.run_events(os.path.join(self.topo_dir, 'simple.events'))
        self.check_gets('1.0.0.1', self.proxyport1, 10, 'proxy.log', 900, 500)
        self.print_log(PROXY_LOG)
        self.check_errors()
        print 'done test_proxy_simple'
    
    def test_proxy_adaptation(self):
        PROXY_LOG = 'proxy.log'
        self.run_proxy(PROXY_LOG, '1', self.proxyport1, '1.0.0.1', '0.0.0.0', '0', '2.0.0.1')
        self.run_events(os.path.join(self.topo_dir, 'adaptation-2000.events')) 
        self.check_gets('1.0.0.1', self.proxyport1, 10, PROXY_LOG, 2000, 1000)
        self.run_events(os.path.join(self.topo_dir, 'adaptation-900.events')) 
        self.check_gets('1.0.0.1', self.proxyport1, 10, PROXY_LOG, 900, 500)
        self.print_log(PROXY_LOG)
        self.check_errors()
        print 'done test_proxy_adaptation'


proj3 = Project3Test()
proj3.setUp()
proj3.test_proxy_simple()

proj3.setUp()
proj3.test_proxy_adaptation()
