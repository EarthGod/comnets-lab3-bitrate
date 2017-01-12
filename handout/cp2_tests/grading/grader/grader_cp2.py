#!/usr/bin/env python

import os
import unittest
from grader_super import Project3Test, emit_scores

TEST_VALUES = {
    'test_dns_rr': 1,
    'test_dns_lsa_topo1': 1,
}

TEST_CATEGORIES = {
    'test_dns_rr': 'dns',
    'test_dns_lsa_topo1': 'dns',
}

class Project3Checkpoint2Test(Project3Test):

    ########### SETUP/TEARDOWN ##########

    # Run once per test suite
    @classmethod
    def setUpClass(cls):
        super(Project3Checkpoint2Test, cls).setUpClass()

    # Run once per test suite
    @classmethod
    def tearDownClass(cls):
        super(Project3Checkpoint2Test, cls).tearDownClass()

    # Run once per test
    def setUp(self):
        super(Project3Checkpoint2Test, self).setUp()

    # Run once per test
    def tearDown(self):
        super(Project3Checkpoint2Test, self).tearDown()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(Project3Checkpoint2Test('test_dns_rr', './topos/rr-dns'))
    suite.addTest(Project3Checkpoint2Test('test_dns_lsa_topo1', './topos/topo1'))
    results = unittest.TextTestRunner(verbosity=2).run(suite)

    emit_scores(results, TEST_VALUES, TEST_CATEGORIES)
